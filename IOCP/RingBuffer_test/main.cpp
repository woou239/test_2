/*
   _head和_tail采用原子变量
   使用共享锁使得多个线程可以访问
   bool add(const long* data, uint32_t size);
   bool remove(long* data, uint32_t size);
   size都为1，每次add或move一列的数据
*/
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <thread>

#define NUM_OF_BOARDS 16				// 板子数量
#define NUM_OF_CHANNELS 16				// 每个板子通道数量
#define ADC_DATA_WIDTH 24				// adc数据位宽
#define SYNC_COPDE_DATA_WIDTH 8			// 同步码位宽
#define SENSOR_485_DATA_WIDTH 64		// 485数据位宽
#define TRANS_FREQUENCY 50000			// 采样率
#define PER_RECV_TIME_COUNT 1000		// 每次tcp接收多少时刻的数据
#define PER_SAMPLE_DATA_LENGTH 1152		// 每个时刻数据的长度(B)

// ringbuffer的width和length
#define RINGBUFFER_WIDTH (NUM_OF_CHANNELS + 2) * NUM_OF_BOARDS
#define RINGBUFFER_LENGTH 4194304 //2的22次方

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
	long _bufferLength;//2的次方数
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

// 后续需要通过不同板子的不同通道改增删数据代码
// 当前为通过_bufferWidth来为不同通道加数据
// size = 1
// data 为某一时刻的所有数据
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

// 取同一时刻的 191 个通道的数据到 data
// 存在问题：存
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

// 直接取 DAT_WIDTH * length 的数据到 data 中
// 存在问题: _head 头指针移动有问题, 每次取一行, ringbuffer
// 此时不能在 remove() 途中读取 _head 指针
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

	// 测试1000*1152B的数据拷贝到ringbuffer中     结果: <=4ms
	for (int i = 0; i < PER_RECV_TIME_COUNT; i++)
	{
		memcpy(trans_rec_data, (buffer + i * PER_SAMPLE_DATA_LENGTH), PER_SAMPLE_DATA_LENGTH);
		ret = m_RingBuffer->add(trans_rec_data, 1);
	}

	
	return 0;
}


