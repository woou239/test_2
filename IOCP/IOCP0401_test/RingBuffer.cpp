#include "RingBuffer.h"
#include <iostream>

/*
* ˵����
* ��ڲ�����
*		buffer_length��				ringbuffer����
*		data_width��				�������ȡ���ݵ�ʱ��ÿ��ʱ��ȡ��������
*/
RingBuffer::RingBuffer(uint32_t buffer_length, uint32_t data_width)
{
	//std::cout << "RingBuffer()" << std::endl;
	//_writeIdx.store(0);
	_writeIdx.store(0);															// ���λ������Ķ�дָ��
	_readIdx.store(0);

	_bufferLength = buffer_length;												// ���λ���������
	_dataWidth = data_width;													
	_bufferSize = buffer_length >> 2;											// remove���ݵ�ʱ�� һ��ʱ��remove���ֽ���(������ȡ��������)
	_bufferLength_div2 = buffer_length >> 1;

	_buffer = new char[buffer_length];											// ringbuffer�ڴ�(һά)
}

/*
* ˵����
* ��������
*/
RingBuffer::~RingBuffer()
{
	//std::cout << "~RingBuffer()" << std::endl;
	delete[] _buffer;
}

/*
* ˵����
* �ж�ringbuffer�Ƿ�Ϊ�գ���дָ��Ͷ�ָ���Ƿ���ͬ������ͬ��Ϊ�ջ�������
*/
bool RingBuffer::isempty()
{
	return (_writeIdx.load() == _readIdx.load());
}

/*
* ˵����
* �ж�ringbuffer�Ƿ�Ϊ������дָ�����ָ���Ƿ�Ϊbuffer����
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
* ��ڲ�����
*		data:					��Ҫ����ringbuffer������
*		size:					data����(�ֽ�)
* ���ڲ�����
*		bool					���״̬
* ˵����
* size��ΪdwBytesTransfered�������ringbuffer��һά��
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
* ��ڲ�����
*		data:					��Ҫȡ�ߵ����ݵ�ַ
*		time_size:				ȡ�߶��ٸ�ʱ�̵�����
* ���ڲ�����
*		bool					ȡ����״̬
* ˵����
* char_size: ȡ��time_size��ʱ�̵����ݵ��ֽڴ�С
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