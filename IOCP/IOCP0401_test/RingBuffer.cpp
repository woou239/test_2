#include "RingBuffer.h"
#include <iostream>

/*
* 说明：
* 入口参数：
*		buffer_length：				ringbuffer长度
*		data_width：				用于算出取数据的时候每个时刻取多少数据
*/
RingBuffer::RingBuffer(uint32_t buffer_length, uint32_t data_width)
{
	//std::cout << "RingBuffer()" << std::endl;
	//_writeIdx.store(0);
	_writeIdx.store(0);															// 环形缓冲区的读写指针
	_readIdx.store(0);

	_bufferLength = buffer_length;												// 环形缓冲区长度
	_dataWidth = data_width;													
	_bufferSize = buffer_length >> 2;											// remove数据的时候 一个时刻remove的字节数(变量名取得有歧义)
	_bufferLength_div2 = buffer_length >> 1;

	_buffer = new char[buffer_length];											// ringbuffer内存(一维)
}

/*
* 说明：
* 析构函数
*/
RingBuffer::~RingBuffer()
{
	//std::cout << "~RingBuffer()" << std::endl;
	delete[] _buffer;
}

/*
* 说明：
* 判断ringbuffer是否为空，即写指针和读指针是否相同，若相同则为空或者满了
*/
bool RingBuffer::isempty()
{
	return (_writeIdx.load() == _readIdx.load());
}

/*
* 说明：
* 判断ringbuffer是否为满，即写指针减读指针是否为buffer长度
*/
bool RingBuffer::isfull()
{
	return ((_writeIdx.load() - _readIdx.load()) == _bufferLength);
}

uint32_t RingBuffer::len()
{
	if (_writeIdx.load() < _readIdx.load())
	{
		return _bufferLength - (_readIdx.load() - _writeIdx.load());
	}
	return _writeIdx.load() - _readIdx.load();
}

uint32_t RingBuffer::remain()
{
	return _bufferLength - len();
}

/*
* 入口参数：
*		data:					需要加入ringbuffer的数据
*		size:					data长度(字节)
* 出口参数：
*		bool					添加状态
* 说明：
* size即为dwBytesTransfered，这里的ringbuffer是一维的
*/
bool RingBuffer::adds(const char* data, uint32_t size)
{
	if (size > remain())
	{
		std::cout << "ERROR:	IOCP Ringbuffer Full" << std::endl;
		return false;
	}
	std::cout << "rev info : " << data << std::endl;

	if(len() > _bufferLength_div2)
	{
		std::cout << "WARNING:	IOCP Ringbuffer Len DIV2" << std::endl;
	}

	uint32_t writeIdx = _writeIdx.load();
	uint32_t remain = _bufferLength - writeIdx;
	if (remain > size)
	{
		memcpy(_buffer + writeIdx, data, size);
		_writeIdx.store(size + writeIdx);
		return true;
	}

	memcpy(_buffer + writeIdx, data, remain);
	memcpy(_buffer, data + remain, size - remain);
	_writeIdx.store(size - remain);
	
	return true;
}

/*
* 入口参数：
*		data:					需要取走的数据地址
*		time_size:				取走多少个时刻的数据
* 出口参数：
*		bool					取数据状态
* 说明：
* char_size: 取出time_size个时刻的数据的字节大小
*/
bool RingBuffer::removes(uint32_t* data, uint32_t time_size)
{
	uint32_t char_size = time_size * _dataWidth;
	if (isempty() || (char_size > len()))
	{
		//std::cout << "WARNING:	IOCP Ringbuffer isempty or not enough" << std::endl;
		return false;
	}

	uint32_t readIdx = _readIdx.load();
	uint32_t remain = _bufferLength - readIdx;
	if (remain > char_size)
	{
		memcpy(data, _buffer + readIdx, char_size);
		_readIdx.store(char_size + readIdx);
		return true;
	}

	memcpy(data, _buffer + readIdx, remain);
	memcpy(reinterpret_cast<char*>(data) + _bufferLength - readIdx, _buffer, char_size - remain);
	_readIdx.store(char_size - remain);
	return true;
}

//bool RingBuffer::remove(uint32_t *data)
//{
//	if (isempty())
//	{
//		//std::cout << "isempty()" << std::endl;
//		return false;
//	}
//	if (_dataWidth > len())
//	{
//		std::cout << "single _dataWidth > len()" << std::endl;
//		return false;
//	}
//	std::cout << "single remove is running" << std::endl;
//	uint32_t readIdx = _readIdx.load();
//	//std::unique_lock<std::shared_mutex> lock(mtx);
//	if (_bufferLength - readIdx > _dataWidth)
//	{
//		memcpy(data, _buffer + readIdx, _dataWidth);
//		//memcpy(data, _buffer + readIdx, 1152);
//		_readIdx.store(_dataWidth + readIdx);
//		return true;
//	}
//
//	memcpy(data, _buffer + readIdx, _bufferLength - readIdx);
//	memcpy(data + ((_bufferLength - readIdx) >> 2), _buffer, _dataWidth - (_bufferLength - readIdx));
//	_readIdx.store(_dataWidth - (_bufferLength - readIdx));
//	return true;
//}

void* memcpy_safe(void* _Destination, int _DestinationSize, const void* _Source, int _SourceSize)
{
	if (_SourceSize > _DestinationSize)
	{
		std::cout << "ERROR:	_SourceSize > _DestinationSize" << std::endl;
		return NULL;
	}

	if ((char*)_Destination >= (char*)_Source + _SourceSize || (char*)_Source >= (char*)_Destination + _SourceSize)
	{
		return memcpy(_Destination, _Source, _SourceSize);
	}
	else
	{
		std::cout << "ERROR:	destination address mix with source address" << std::endl;
		return NULL;
	}
}