#include <atomic>
#include <iostream>
#include <string>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <chrono>
#include <thread>
#include "Windows.h"

#include "singleton.h"
#include "threadpool.h"

using namespace std;

#define PER_SAMPLE_DATA_LENGTH 1152
#define ALTITUDE_SENSOR 180/32768

#define CMD_CONNECT_4NODE_25K_SAMPLE_RATE			(1, 4, 1)
#define CMD_CONNECT_11NODE_25K_SAMPLE_RATE			(1, 11, 1)
#define CMD_CONNECT_4NODE_50K_SAMPLE_RATE			(1, 4, 2)
#define CMD_CONNECT_11NODE_50K_SAMPLE_RATE			(1, 11, 2)
#define CMD_CONNECT_4NODE_100K_SAMPLE_RATE			(1, 4, 3)
#define CMD_CONNECT_11NODE_100K_SAMPLE_RATE			(1, 11, 3)

#define CMD_SELFCHECK								(2, 11, 2)
#define CMD_BEGIN									(3, 11, 2)

double buffer[1000];

//mutex mtx;
//condition_variable cv;
//vector<int> data_vector;
//
//
//void Producer()
//{
//	while (1)
//	{
//		unique_lock<mutex> lck(mtx);
//		data_vector.push_back(20);
//
//		if (data_vector.size() >= 20)
//		{
//			cv.notify_one();
//		}	
//	}
//
//}
//
//void Consumer()
//{
//	while (1)
//	{
//		int num[20] = { 0 };
//		unique_lock<mutex> lck(mtx);
//		cv.wait(lck, [] {return !data_vector.empty(); });
//		for (int i = 0; i < 20; i++)
//		{
//			num[i] = data_vector.back();
//			data_vector.pop_back();
//		}
//		cout << "num" << " Consumer ing" << endl;
//	}
//
//}

bool fun1()
{
	for (int i = 0; i < 0x00ffffff; i++)
	{
		//for (int i = 0; i < 0x00ffffff; i++)
		//{

		//}
	}
	cout << "fun1 is running" << endl;
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
}

bool TransForm_altitude_sensor(double data_low, double data_high, double* altitude_sensor_data)
{
	//std::cout <<std::hex <<data_low << std::endl;
	//std::cout << std::hex << data_high << std::endl;
	if (((uint32_t)data_high >> 24) != 0x01)
	{
		return false;
	}

	altitude_sensor_data[0] = TransForm_16(((uint32_t)data_high) & 0xffff) * ALTITUDE_SENSOR;
	altitude_sensor_data[1] = TransForm_16(((uint32_t)data_low) >> 16) * ALTITUDE_SENSOR;
	altitude_sensor_data[2] = TransForm_16(((uint32_t)data_low) & 0xffff) * ALTITUDE_SENSOR;
	return true;
}

struct CMD {
	char _type_of_cmd;
	char _num_of_node;
	char _sample_rate;
	char _result_of_selfcheck[4];
	char _null[3]; //没有用到
};



void set_cmd_type(char type_of_cmd, char num_of_node, char sample_rate, CMD &command)
{
	command._type_of_cmd = type_of_cmd;
	command._num_of_node = num_of_node;
	command._sample_rate = sample_rate;
}



int main()
{
	CMD cmd;
	set_cmd_type(1,3,2, cmd);

	//uint32_t num_32_h = 0x010000a3;
	//uint32_t num_32_l = 0x40004000;

	//double num[2] = { 0 };
	//num[1] = num_32_h;
	//num[0] = num_32_l;

	//double altitude_sensor_data[3] = { 0 };

	//TransForm_altitude_sensor(num[0], num[1], altitude_sensor_data);
	//std::cout << altitude_sensor_data[0] << std::endl;
	//std::cout << altitude_sensor_data[1] << std::endl;
	//std::cout << altitude_sensor_data[2] << std::endl;


	//std::cout << strlen("1") << std::endl;

	// 测试二维数组内存中的存放
	//uint32_t** data;
	//data = new  uint32_t * [5];
	//for (int i = 0; i < 5; i++)
	//{
	//	data[i] = new uint32_t[1152];
	//}
	////for (int i = 0; i < 4; i++)
	////{
	////	for (int j = 0; j < 6; j++)
	////	{
	////		data[i][j] = j;
	////	}
	////}
	//std::cout << "data address: " << *(data+1) << std::endl;
	//for (int i = 0; i < 4; i++)
	//{
	//	for (int j = 0; j < 6; j++)
	//	{
	//		cout << data[i][j] << " ";
	//	}
	//	cout << endl;
	//}
	//cout << *(*(data + 3) + 1) << endl;



	//std::threadpool executor{ 12 };

	//future<bool> ff = executor.commit(fun1);
	//for (int i = 0; i < 0x00ffffff; i++)
	//{
	//	for (int i = 0; i < 0xff; i++)
	//	{

	//	}
	//}
	//
	////SYSTEM_INFO si;
	////GetSystemInfo(&si);
	////int m_nProcessors = si.dwNumberOfProcessors;

	////cout << "m_nProcessors" << m_nProcessors << endl;
	//if (ff.get())
	//{
	//	cout << "ff.get(): " << endl;
	//}
	
	//thread pro(Producer);
	//thread con(Consumer);
	//pro.join();
	//con.join();
	


	//is_lock_free()
	// bool类型的原子变量用：atomic_flag
	//std::atomic_flag bool_flag = ATOMIC_FLAG_INIT;
	//std::cout << "atomic_flag: " << bool_flag.test_and_set() << std::endl;
	//bool_flag.clear();
	//std::cout << "atomic_flag: " << bool_flag.test_and_set() << std::endl;
	//std::atomic<int> int_atomic;
	//std::cout << "int_atomic.is_lock_free(): " << int_atomic.is_lock_free() << std::endl;

	//Singleton_3 s = Singleton_3::getInstance();
	//cout << "Singleton_3 address: " << s.flag << endl;

	//Singleton_3 s1 = Singleton_3::getInstance();
	//cout << "Singleton_3 address: " << s1.flag << endl;

	//int idx = 0;
	//for (int i = 0; i < 1000; i++)
	//{
	//	buffer[i] = i;
	//}
	//for (int i = 0; i < 1000; i++)
	//{
	//	if ((i & (255)) == 0)
	//	{
	//		std::cout << *(buffer + idx * 256) << std::endl;
	//		idx++;
	//	}
	//	
	//}
	//std::cout << *(buffer + 255) << std::endl;
	//std::cout << *(buffer + idx * 256) << std::endl;
	//std::cout << *(buffer + idx * 256) << std::endl;
	//std::cout << *(buffer + idx * 256) << std::endl;

	//std::cout << sizeof(long long) << std::endl;


	system("pause");
	return 0;
}