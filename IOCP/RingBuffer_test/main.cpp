/*
   _head��_tail����ԭ�ӱ���
   ʹ�ù�����ʹ�ö���߳̿��Է���
   bool add(const long* data, uint32_t size);
   bool remove(long* data, uint32_t size);
   size��Ϊ1��ÿ��add��moveһ�е�����
*/
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <thread>

#define NUM_OF_BOARDS 16				// ��������
#define NUM_OF_CHANNELS 16				// ÿ������ͨ������
#define ADC_DATA_WIDTH 24				// adc����λ��
#define SYNC_COPDE_DATA_WIDTH 8			// ͬ����λ��
#define SENSOR_485_DATA_WIDTH 64		// 485����λ��
#define TRANS_FREQUENCY 50000			// ������
#define PER_RECV_TIME_COUNT 1000		// ÿ��tcp���ն���ʱ�̵�����
#define PER_SAMPLE_DATA_LENGTH 1152		// ÿ��ʱ�����ݵĳ���(B)

// ringbuffer��width��length
#define RINGBUFFER_WIDTH (NUM_OF_CHANNELS + 2) * NUM_OF_BOARDS
#define RINGBUFFER_LENGTH 4194304 //2��22�η�

#define min(a, b) (a < b? a : b)



std::mutex m_mtx;
std::condition_variable cv;

class RingBuffer
{
public:
	RingBuffer(int width, int length);
	~RingBuffer();

	bool isempty();
	bool isfull();
	uint32_t remain();
	uint32_t len();
	bool add(const long* data, uint32_t size);
	bool remove(float* data, uint32_t size);
	bool remove(float** data, uint32_t size, uint32_t length);
	long** _buffer;

private:
	std::shared_mutex mtx;
	std::atomic<volatile uint32_t> _head;
	std::atomic<volatile uint32_t> _tail;
	//volatile uint32_t _head;
	//volatile uint32_t _tail;
	int _bufferWidth;
	long _bufferLength;//2�Ĵη���
};

RingBuffer::RingBuffer(int width, int length)
{
	_head.store(0);
	_tail.store(0);
	_bufferWidth = width;
	_bufferLength = length;
	_buffer = new long* [_bufferWidth];
	for (int i = 0; i < _bufferWidth; i++)
	{
		_buffer[i] = new long[_bufferLength];
	}
}

RingBuffer::~RingBuffer()
{
	for (int i = 0; i < _bufferWidth; i++)
	{
		delete[] _buffer[i];
	}
	delete[] _buffer;
}

bool RingBuffer::isempty()
{
	uint32_t head, tail;
	head = _head.load();
	tail = _tail.load();
	return (head == tail);
}

bool RingBuffer::isfull()
{
	return ((_tail.load() - _head.load()) == _bufferLength);
}

uint32_t RingBuffer::remain()
{
	return _bufferLength - (_tail.load() - _head.load());
}

uint32_t RingBuffer::len()
{
	return _tail.load() - _head.load();
}

// ������Ҫͨ����ͬ���ӵĲ�ͬͨ������ɾ���ݴ���
// ��ǰΪͨ��_bufferWidth��Ϊ��ͬͨ��������
// size = 1
// data Ϊĳһʱ�̵���������
bool RingBuffer::add(const long* data, uint32_t size)
{
	if (size > remain())
	{
		std::cout << "ringbuffer is full, but still add data." << std::endl;
		return false;
	}

	//std::shared_lock<std::shared_mutex> lock(mtx);
	{
		for (int j = 0; j < _bufferWidth; j++)
		{
			*(*(_buffer + j) + (_tail.load() & (_bufferLength - 1))) = data[j];
		}
	}
	_tail.fetch_add(size);
	return 0;
}

// ȡͬһʱ�̵� 191 ��ͨ�������ݵ� data
// �������⣺��
bool RingBuffer::remove(float* data, uint32_t size)
{
	if (isempty() && (size > len()))
	{
		std::cout << "ringbuffer is empty, but still get data." << std::endl;
		return false;
	}
	//std::unique_lock<std::shared_mutex> lock(mtx);
	{
		for (int j = 0; j < _bufferWidth; j++)
		{
			data[j] = *(*(_buffer + j) + (_head.load() & (_bufferLength - 1)));

		}
	}
	_head.fetch_add(size);
	return true;
}

// ֱ��ȡ DAT_WIDTH * length �����ݵ� data ��
// ��������: _head ͷָ���ƶ�������, ÿ��ȡһ��, ringbuffer
// ��ʱ������ remove() ;�ж�ȡ _head ָ��
bool RingBuffer::remove(float** data, uint32_t size, uint32_t length)
{
	if (isempty() && (size > len()))
	{
		std::cout << "ringbuffer is empty, but still get data." << std::endl;
		return false;
	}
	//std::unique_lock<std::shared_mutex> lock(mtx);
	{
		for (int j = 0; j < _bufferWidth; j++)
		{
			for (uint32_t k = 0; k < length; k++)
			{
				data[j][k] = *(*(_buffer + j) + (_head.load() & (_bufferLength - 1)));
				_head.fetch_add(size);
			}
			_head.fetch_sub(length);
		}
	}
	return true;
}

//void test_thread_read(RingBuffer* rb)
//{
//	long* data = new long[4];
//	while (1)
//	{
//		while (!rb->isempty())
//		{
//			rb->remove(data, 1);
//			std::cout << "read data: " << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << std::endl;
//	
//		}
//	}
//	delete[] data;
//}
//
//void test_thread_write(RingBuffer* rb)
//{
//	long* data = new long[4];
//	for (int i = 0; i < 4; i++)
//	{
//		data[i] = i;
//	}
//	
//	while (1)
//	{
//		while (!rb->isfull())
//		{
//			rb->add(data, 1);
//		}
//	}
//	delete[] data;
//}

RingBuffer* m_RingBuffer;

int main()
{
	m_RingBuffer = new RingBuffer(RINGBUFFER_WIDTH, RINGBUFFER_LENGTH);
	long* trans_rec_data=new long[PER_SAMPLE_DATA_LENGTH >> 2];
	char* buffer = new char[PER_RECV_TIME_COUNT * PER_SAMPLE_DATA_LENGTH];
	for (int i = 0; i < PER_RECV_TIME_COUNT * PER_SAMPLE_DATA_LENGTH; i++)
	{
		buffer[i] = i % 200;
	}
	bool ret = false;

	// ����1000*1152B�����ݿ�����ringbuffer��     ���: <=4ms
	for (int i = 0; i < PER_RECV_TIME_COUNT; i++)
	{
		memcpy(trans_rec_data, (buffer + i * PER_SAMPLE_DATA_LENGTH), PER_SAMPLE_DATA_LENGTH);
		ret = m_RingBuffer->add(trans_rec_data, 1);
	}

	
	return 0;
}


