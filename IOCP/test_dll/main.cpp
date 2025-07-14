#include <iostream>
#include <chrono>
#include <string>
#include <mutex>
#include <Windows.h>
#include "IOCP_DLL.h"
using namespace std;

#pragma comment(lib,"IOCP0401_test.lib")

#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)

FILE* this_file = nullptr;
std::condition_variable cv_saveFile;

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args) {
	size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);
	char* bytes = new char[size];
	snprintf(bytes, size, format.c_str(), args ...);
	std::string res = std::string(bytes);
	delete[] bytes;
	return res;
}

void local_time(std::string& time)
{
	tm _localTime;
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	localtime_r(&in_time_t, &_localTime);
	//if(_localTime.tm_mon + 1 < 10)
	time = std::to_string(_localTime.tm_year + 1900) + '-'
		+ string_format(std::to_string(_localTime.tm_mon + 1), "%02d") + '-'
		+ string_format(std::to_string(_localTime.tm_mday), "%02d") + ' '
		+ string_format(std::to_string(_localTime.tm_hour), "%02d") + '-'
		+ string_format(std::to_string(_localTime.tm_min), "%02d") + '-'
		+ string_format(std::to_string(_localTime.tm_sec), "%02d");
}

void set_file_name(char path[])
{

	std::string file_name;
	std::string save_dir_path = path;
	local_time(file_name);

	file_name = file_name + ".dat";

	file_name = "  " + file_name;

	file_name = save_dir_path + "\\" + file_name;

#ifdef COUT_DEBUG
	std::cout << file_name << std::endl;
#endif

	this_file = fopen(file_name.c_str(), "wb+");
}



int main(void)
{
	int cnt = 0;
	char s[] = "nihao";
	//char channel_map_path[] = "C:\\Users\\Moon\\Desktop\\20241125";
	char channel_map_path[] = "D:\\Project\\learnC++\\QT_PRO\\DigitalArrays\\data";
	//char channel_map_path1[] = "D:\\Project\\learnC++\\QT_PRO\\DigitalArrays\\data1";
	bool ret = false;
	uint32_t* buffer = new uint32_t[512 * 198];
	uint32_t* save_buffer = new uint32_t[198 * 50000 * 10];
	init(11, s, channel_map_path, 10, 50000);
	begin_save(false);
	Sleep(5000);
	//send_msg(2);
	Sleep(1000);
	//send_msg(3);



	while (1)
	{

		if (get_data(buffer, 512))
		{
			//std::cout<< hex <<buffer[0] <<std::endl;
		//	//memcpy(save_buffer + cnt * (500 * 198), buffer, 500 * 198 * 4);
		//	//cnt++;
		//	//if (cnt == 1000)
		//	//{
		//	//	set_file_name(channel_map_path1);
		//	//	fwrite(save_buffer, sizeof(uint32_t), 11*18*50000*10, this_file);
		//	//	fclose(this_file);
		//	//	cnt = 0;
		//	//}
		}


	}

	//char buf[4] = { 0x10,0x20,0x30,0x40 };
	//int num = 0;
	//memcpy(&num, buf, 4);
	//cout << num << endl;

	return 0;
}