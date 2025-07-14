#include "RingBuffer.h"
#include <iostream>
#include <mutex>

double bias[180] = { 
	0,0.0001714465,0.0005184601,0.0002451658,0.0002760571,0.0002035483,-0.0006339436,-0.0002814085,0,0.0000428367,-0.0000328553,0.0007449327,0.0008237953,0.0005936101,-0.0003200174,-0.0003929998,0,0,
	0.0004877323,-0.0002666987,0.0000469189,0.0002733301,0.0002179160,0.0000919171,0.0002578797,0.0000215406,-0.0241098169,0.0000379758,0.0006043671,0.0004107536,0.0001958551,-0.0006909075,0.0002432710,-0.0001418039,0,0,
	-0.0002626228,-0.0002308712,-0.0002816050,-0.0001098306,0.0004526461,0.0070122206,0.0003430030,-0.0000471931,0.0001583740,0.0003897259,-0.0001375041,0.0004986543,0.0008006285,0.0002299461,0.0009889399,0.0002656755,0,0,
	0.0005099321, 	0.0012334191, - 0.0000605521, - 0.0000077872, 	0.0005504573, 	0.0006564595, 	0.0006286300, - 0.0000660192, 	0.0002176950, - 0.0006203294, 	0.0007998215, 	0.0002579487, - 0.0000099452, 	0.0004956877, 	0.0009734000, - 0.0005264942,0,0,
	-0.0000202545, - 0.0002214530, 	0.0005850949, 	0.0002216306, 	0.0006565466, 	0.0002547791, 	0.0004081120, 	0.0002763645, 	0.0007750690, 	0.0004400558, 	0.0004617125, 	0.0004980822, - 0.0000102518, 	0.0002483934, - 0.0000303806, 	0.0001430431,0,0,
	0.0005636079, 	0.0010036404, - 0.0000058391, 	0.0002018341, 	0.0001806677, 	0.0008149046, - 0.0001166557, - 0.0025275901, - 0.0005588993, 	0.0004781372, - 0.0009482686, 	0.0001811487, 	0.0005285443, 	0.0000432672, - 0.0003854957, 	0.0009077290,0,0,
	0.0006843170, - 0.0004571176, 	0.0008425827, 	0.0005102092, 	0.0001991245, 	0.0010446348, 	0.0002256289, 	0.0001406161, - 0.0001005095, 	0.0004265828, 	0.0002827929, - 0.0000568609, 	0.0004151972, 	0.0005642403, - 0.0001046728, 	0.0001960125,0,0,
	0.0002607661, 	0.0003293235, 	0.0004605151, 	0.0000229108, 	0.0000389886, 	0.0000447151, 	0.0007371299, 	0.0002803152, - 0.0000030721, 	0.0002092806, 	0.0001519949, - 0.0001609239, 	0.0006438461, - 0.0002408475, - 0.0000586579, - 0.0001763939,0,0,
	0.0010236356, 	0.0002643002, 	0.0003518289, 	0.0007392113, - 0.0000777720, 	0.0000715009, - 0.0003864345, - 0.0003009992, 	0.0003611792, 	0.0019168226, - 0.0000168213, 	0.0001703664, 	0.0011299523, - 0.0001296369, - 0.0005165760, 	0.0002762615,0,0,
	-0.0010032543, - 0.0003034218, 	0.0007718450, 	0.0002922883, - 0.0001168096, 	0.0069904306, 	0.0008557272, 	0.0002926170, - 0.0002336480, 	0.0005012849, 	0.0005707136, - 0.0004760851, 	0.0004701188, - 0.0002362602, 	0.0000839493, 	0.0001079634,0,0
};

// buffer_length: ringbuffer长度
// data_width: 某一时刻有多少个通道的数据(将485的64位数据视为两个通道)
// buffer_length: ringbuffer的长度
// buffer_width: ringbuffer的宽度 = 通道数 * 18
// perGetNum: 每次从ringbuffer取的数据长度，用于送给显示
RingBuffer::RingBuffer(uint32_t buffer_length, uint32_t buffer_width, uint32_t perGetNum)
	:_bufferLength{ buffer_length }, _bufferWidth{ buffer_width }, _perGetNum{ perGetNum }
{
	//_writeIdx.store(0);
	_writeIdx = 0;
	_readIdx = 0;
	_bufferLength_div2 = buffer_length >> 1;

	_buffer = new double * [buffer_width];
	for (int i = 0; i < buffer_width; i++)
	{
		_buffer[i] = new double[buffer_length];
	}

#ifdef ADD_SYNC_CODE
	_sync_code_buffer = new int[buffer_length];
	now_sync_code = 0;
	last_sync_code = 0;
#endif

}

RingBuffer::~RingBuffer()
{
	for (int i = 0; i < _bufferWidth; i++)
	{
		delete[] _buffer[i];
	}
	delete[] _buffer;

#ifdef ADD_SYNC_CODE
	delete[] _sync_code_buffer;
#endif
}

bool RingBuffer::isempty()
{
	return (_writeIdx.load() == _readIdx.load());
}

bool RingBuffer::isfull()
{
	return ((_writeIdx.load() - _readIdx.load()) == _bufferLength-1);
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
* 一次加1个时刻所有通道的数据 修改后将485数据的两个32位合成了一个64位存储到一个double类型中，后期再还原
*/
bool RingBuffer::adds(const uint32_t* data)
{
	if (isfull() == true)
	{
		std::cout << "ERROR:	QT Ringbuffer Full" << std::endl;
		return false;
	}
	if (len() > _bufferLength_div2)
	{
		std::cout << "WARNING:	QT Ringbuffer Len DIV2" << std::endl;
	}
	uint32_t writeIdx = _writeIdx.load();
	for (int i = 0; i < _bufferWidth; i++)
	{
		if (((i + 1) % 18 == 17) || ((i + 1) % 18 == 0))
		{
			_buffer[i][writeIdx] = data[i];
		}
		else
		{
			//_buffer[i][writeIdx] = REF_LISTENER * TransForm(data[i]);
			_buffer[i][writeIdx] = REF_LISTENER * TransForm(data[i]) - bias[i];
		}
	}

	writeIdx = (writeIdx == _bufferLength) ? 0 : writeIdx + 1;
	_writeIdx.store(writeIdx);
	return true;
}

/*
* show_time_size: 实时显示的数据长度 因为memcpy移动的是字节数，要显示show_time_size这么多的数据，每个数据是double类型8个字节，所以要乘以8，即移动这么多字节
*/
bool RingBuffer::removes(double** data, int show_time_size)
{
	if (isempty() || (show_time_size > len()))
	{
		//std::cout << "WARNING:	QT Ringbuffer isempty or not enough" << std::endl;
		return false;
	}

	uint32_t readIdx = _readIdx.load();
	if (_bufferLength - readIdx > show_time_size)
	{
		for (int i = 0; i < _bufferWidth; i++)
		{
			memcpy(data[i], _buffer[i] + readIdx, show_time_size << 3);
		}
		//memcpy(data, _buffer + readIdx, 1152);
		_readIdx.store(_perGetNum + readIdx);
		return true;
	}

	for (int i = 0; i < _bufferWidth; i++)
	{
		memcpy(data[i], _buffer[i] + readIdx, (_bufferLength - readIdx) << 3);
		memcpy(data[i] + (_bufferLength - readIdx), _buffer[i], (show_time_size - (_bufferLength - readIdx)) << 3);
	}

	if (_bufferLength - readIdx < _perGetNum)
	{
		readIdx = _perGetNum - (_bufferLength - readIdx);
	}
	else
	{
		readIdx = _perGetNum + readIdx;
	}
	_readIdx.store(readIdx);
	return true;
}

int trans2Idx(int board, int channel)
{
	int ret = (board - 1) * 18 + channel - 1;
	if ((ret < 0) || (ret > 11 * 18))
	{
		std::cout << "idx error" << std::endl;
		return 0;
	}
	return ret;
}

long TransForm(unsigned long org)
{
	long res = org & 0x00ffffff;
	//unsigned long channel = org & 0x07000000;
	//long res = org - channel;
	if (res > 0x01000000)
	{
		;
		std::cout << "error";
	}
	else if (res >= 0x00800000 && res < 0x01000000)
	{
		res = 0x01000000 - res;
		res = res * -1;
		return res;
	}
	else if (res < 0x00800000 && res >= 0)
	{
		return res;
	}
	else
	{
		;
		std::cout << res;
	}
	return 0;
}

// data_low是第17个32位数据，对应低32位
// data_high是第18个32位数据，对应高32位
// altitude_sensor_data 传入的姿态传感器指针
bool TransForm_altitude_sensor(double data_low, double data_high, double* altitude_sensor_data)
{
	//std::cout <<std::hex << (uint32_t)data_high << std::endl;
	//std::cout << std::hex << (uint32_t)data_low << std::endl;

	if (((uint32_t)data_high & 0xff) != 0x01)
	{
		return false;
	}
	// 收到的data_high相对于协议而言是高位在后，低位在前的，因此先trans
	altitude_sensor_data[0] = TransForm_16(trans(((uint32_t)data_high) >> 16)) * ALTITUDE_SENSOR;
	altitude_sensor_data[1] = TransForm_16(trans(((uint32_t)data_low) & 0xffff)) * ALTITUDE_SENSOR;
	altitude_sensor_data[2] = TransForm_16(trans(((uint32_t)data_low) >> 16)) * ALTITUDE_SENSOR;
	return true;
}

bool TransForm_angle_sensor(double data_low, double data_high, double* angle_sensor_data)
{
	if (((uint32_t)data_high & 0xff) != 0x02)
	{
		return false;
	}
	angle_sensor_data[0] = ((((uint32_t)data_low) >> 16) - ANGLE_OFFSET) * ANGLE_SENSOR;
	angle_sensor_data[1] = ((((uint32_t)data_low) & 0xffff) - ANGLE_OFFSET) * ANGLE_SENSOR;
	return true;
}

const double press_point[4] = { 1, 0.1, 0.01, 0.001 };

bool TransForm_press_sensor(double data_low, double data_high, double *press_sensor_data, int *symbol)
{
	if (((uint32_t)data_high & 0xff) != 0x03)
	{
		return false;
	}
	int point = (uint32_t)data_low & 0xff;
	*symbol = ((uint32_t)data_low >> 8) & 0xff;
	if (*symbol > 2)
	{
		*symbol = 1;
		return false;
	}

	*press_sensor_data = TransForm_16(trans(((uint32_t)data_low) >> 16)) * press_point[point];
	//std::cout << *press_sensor_data << std::endl;
	return true;
}

int TransForm_16(uint32_t org)
{
	long res = org & 0x0000ffff;
	if (res > 0x00010000)
	{
		std::cout << "error";
	}
	else if (res >= 0x00008000 && res < 0x00010000)
	{
		res = 0x00010000 - res;
		res = res * -1;
		return res;
	}
	else if (res < 0x00008000 && res >= 0)
	{
		return res;
	}
	else
	{
		std::cout << "error" << std::endl;
	}
}

int trans(uint32_t org)
{
	uint32_t temp1 = org >> 8; // 高16位
	return ((org & 0xff) << 8) | temp1;
}

uint32_t trans_selfcheck_state_for_qt(uint32_t org)
{
	uint32_t high8_1 = (org & 0xff) << 24;
	uint32_t high8_2 = ((org >> 8) & 0xff) << 16;
	uint32_t low8_2 = ((org >> 16) & 0xff) << 8;
	uint32_t low8_1 = (org >> 24) & 0xff;
	return high8_1 | high8_2 | low8_2 | low8_1;
}


bool RingBuffer::adds_with_sync_code(const uint32_t* data)
{
	if (isfull() == true)
	{
		std::cout << "ERROR:	QT Ringbuffer Full" << std::endl;
		return false;
	}
	if (len() > _bufferLength_div2)
	{
		std::cout << "WARNING:	QT Ringbuffer Len DIV2" << std::endl;
	}

	uint32_t writeIdx = _writeIdx.load();

#ifdef ADD_SYNC_CODE
	_sync_code_buffer[writeIdx] = data[0] >> 24;
#endif

	for (int i = 0; i < _bufferWidth; i++)
	{
		if (((i + 1) % 18 == 17) || ((i + 1) % 18 == 0))
		{
			_buffer[i][writeIdx] = data[i];
		}
		else
		{
			/*_buffer[i][writeIdx] = REF_LISTENER * TransForm(data[i]);*/
			_buffer[i][writeIdx] = REF_LISTENER * TransForm(data[i]) - bias[i];
		}
	}

	writeIdx = (writeIdx == _bufferLength) ? 0 : writeIdx + 1;
	_writeIdx.store(writeIdx);
	return true;
}

bool RingBuffer::removes_with_sync_code(double** data, int show_time_size)
{
	if (isempty() || (show_time_size > len()))
	{
		//std::cout << "WARNING:	QT Ringbuffer isempty or not enough" << std::endl;
		return false;
	}

	uint32_t readIdx = _readIdx.load();

#ifdef ADD_SYNC_CODE
	last_sync_code = _sync_code_buffer[readIdx];
#endif
	
	if (_bufferLength - readIdx > show_time_size)
	{
		for (int i = 0; i < _bufferWidth; i++)
		{
			memcpy(data[i], _buffer[i] + readIdx, show_time_size << 3);
		}
		readIdx = readIdx + _perGetNum;

#ifdef ADD_SYNC_CODE
		now_sync_code = _sync_code_buffer[readIdx];
		if (now_sync_code != last_sync_code)
		{
			_readIdx.store(readIdx);

			std::cout << "ERROR:	SYNC_CODE_ERROR : " << (now_sync_code - last_sync_code) << std::endl;
			return false;
		}
#endif

		_readIdx.store(readIdx);
		return true;
	}

	for (int i = 0; i < _bufferWidth; i++)
	{
		memcpy(data[i], _buffer[i] + readIdx, (_bufferLength - readIdx) << 3);
		memcpy(data[i] + (_bufferLength - readIdx), _buffer[i], (show_time_size - (_bufferLength - readIdx)) << 3);
	}

	if (_bufferLength - readIdx < _perGetNum)
	{
		readIdx = _perGetNum - (_bufferLength - readIdx);
	}
	else
	{
		readIdx = _perGetNum + readIdx;
	}

#ifdef ADD_SYNC_CODE
	now_sync_code = _sync_code_buffer[readIdx];
	if (now_sync_code != last_sync_code)
	{
		_readIdx.store(readIdx);
		std::cout << "ERROR:	SYNC_CODE_ERROR : " << (now_sync_code - last_sync_code) << std::endl;
		return false;
	}
#endif
	
	_readIdx.store(readIdx);
	return true;
}
