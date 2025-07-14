#pragma once
#include <shared_mutex>

//#define REF_LISTENER (1.1111*0.000000488292*1.25)
#define REF_LISTENER (1.1111*0.000000488292*1.25)
#define ALTITUDE_SENSOR 0.0054931640625 //180/32768
#define ANGLE_SENSOR 0.01
#define ANGLE_OFFSET 9000

//#define ADD_SYNC_CODE 
//#define NODE_18
#define NODE_224

#define NODE_4_ADAPTIVE_SAMPLE 1
#define NODE_11_50K_SAMPLE 1

class RingBuffer
{
public:
	RingBuffer(uint32_t buffer_length, uint32_t data_width, uint32_t perGetNum);
	~RingBuffer();

	bool isempty();
	bool isfull();
	uint32_t remain();
	uint32_t len();
	bool adds(const uint32_t* data);
	bool removes(double** data, int show_time_size);

	double** _buffer;

	bool adds_with_sync_code(const uint32_t* data);
	bool removes_with_sync_code(double** data, int show_time_size);
	int* _sync_code_buffer;

private:
	std::shared_mutex mtx;
	std::atomic<volatile uint32_t> _writeIdx;
	std::atomic<volatile uint32_t> _readIdx;
	uint32_t _bufferLength;									// 2的次方数 
	uint32_t _bufferWidth;									// 11*18
	uint32_t _perGetNum;									// 2048
	uint32_t _bufferLength_div2;

	int now_sync_code;
	int last_sync_code;
};

int trans2Idx(int board, int channel);
long TransForm(unsigned long org);
bool TransForm_altitude_sensor(double data_low, double data_high, double* altitude_sensor_data);
bool TransForm_angle_sensor(double data_low, double data_high, double* angle_sensor_data);
bool TransForm_press_sensor(double data_low, double data_high, double* press_sensor_data, int *symbol);
int TransForm_16(uint32_t org);
int trans(uint32_t org);
uint32_t trans_selfcheck_state_for_qt(uint32_t org);

