#pragma once
#include <shared_mutex>

// �����Ƶ�ringbuffer�Ƕ�ά�ģ�����Ϊ����Ч�ʲ��ߣ���Ϊƽ�̵�ringbuffer

class RingBuffer
{
public:
	RingBuffer(uint32_t buffer_length, uint32_t data_width);
	~RingBuffer();

	bool isempty();
	bool isfull();
	uint32_t remain();
	uint32_t len();
	bool adds(const char* data, uint32_t size);
	bool removes(uint32_t* data, uint32_t time_size);
	bool remove(uint32_t *data);
	char* _buffer;

private:
	std::shared_mutex mtx;
	std::atomic<volatile uint32_t> _writeIdx;
	std::atomic<volatile uint32_t> _readIdx;
	uint32_t _bufferLength;									// 2�Ĵη��� 2���ĳ���
	uint32_t _bufferSize;
	uint32_t _dataWidth;									// 1152
	uint32_t _bufferLength_div2;
};

void* memcpy_safe(void* _Destination, int _DestinationSize, const void* _Source, int _SourceSize);


