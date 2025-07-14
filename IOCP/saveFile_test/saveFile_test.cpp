#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <fstream>
#include <iomanip>
#include "threadpool.h"
#include <io.h>

#define BUFFER_LEN 500000
#define BUFFER_WIDTH 72


#define REF_LISTENER (1.1111*0.000000488292)
#define SAMPLE_RATE 50000

void test_fwriteDAT()
{
	uint32_t** bufff = new uint32_t * [BUFFER_WIDTH];
	for (int i = 0; i < BUFFER_WIDTH; i++)
	{
		bufff[i] = new uint32_t[BUFFER_LEN];
		std::fill(*(bufff + i), *(bufff + i) + BUFFER_LEN, i);
	}

	auto start = std::chrono::system_clock::now();
	std::string FileName = std::to_string(1) + "csvTest_fprintf.dat";
	FILE* fp = fopen(FileName.c_str(), "wb");
	if (fp == NULL)
	{
		std::cout << "fopen error" << std::endl;
	}
	unsigned int returnNum;
	for (int i = 0; i < BUFFER_WIDTH; i++)
	{
		returnNum = fwrite(bufff[i], sizeof(uint32_t), BUFFER_LEN, fp);
	}
	
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "花费了"
		<< double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << std::endl;
	fclose(fp);
}

//// 1.0
//void test_readDAT(std::string FileName)
//{
//	uint32_t** bufff = new uint32_t * [BUFFER_WIDTH];
//	for (int i = 0; i < BUFFER_WIDTH; i++)
//	{
//		bufff[i] = new uint32_t[BUFFER_LEN];
//	}
//
//	std::string name = FileName;
//	name = name.substr(0, name.length() - 4) + ".csv";
//
//	FILE* fp = fopen(FileName.c_str(), "rb");
//	FILE* fp1 = fopen(name.c_str(), "w+");
//
//	for (int i = 0; i < BUFFER_WIDTH; i++)
//	{
//		fread(bufff[i], sizeof(uint32_t), BUFFER_LEN, fp);
//	}
//	fclose(fp);
//
//	auto start = std::chrono::system_clock::now();
//	for (int i = 0; i < BUFFER_LEN; ++i)
//	{
//		for (int j = 0; j < BUFFER_WIDTH; j++)
//		{
//			fprintf(fp1, "%f,", ((bufff[j][i])));
//		}
//		fprintf(fp1, "\n");
//	}
//	fclose(fp1);
//}

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

// test1.0
void test_readDAT(std::string FileName, int board_num, int save_num)
{
	int cnt_cout = 6;

	uint32_t last_data = 0;
	uint32_t new_data = 0;

	int bufferLen = save_num * SAMPLE_RATE;
	int bufferWidth = board_num * 18;

	uint32_t** bufff = new uint32_t * [bufferLen];
	for (int i = 0; i < bufferLen; i++)
	{
		bufff[i] = new uint32_t[bufferWidth];
	}

	std::string name = FileName;
	name = name.substr(0, name.length() - 4) + ".csv";

	FILE* fp = fopen(FileName.c_str(), "rb");
	FILE* fp1 = fopen(name.c_str(), "w+");

	for (int i = 0; i < bufferLen; i++)
	{
		fread(bufff[i], sizeof(uint32_t), bufferWidth, fp);
	}
	fclose(fp);

	auto start = std::chrono::system_clock::now();
	for (int i = 0; i < bufferLen; ++i)
	{
		new_data = bufff[i][0] >> 24;
		if ((new_data == 0) && (last_data == 255))
		{

		}
		else
		{
			if ((new_data - last_data) != 1)
			{
				if (cnt_cout > 0)
				{
					std::cout << i << std::endl;
					cnt_cout--;
				}
				
				if (i != 0)
				{
					//break;
				}
			}
		}
		last_data = new_data;
		for (int j = 0; j < bufferWidth; j++)
		{
			fprintf(fp1, "%lf,", (REF_LISTENER * TransForm(bufff[i][j])));
			/*std::cout << (bufff[i][j] & 0xff) << std::endl;*/
		}
		fprintf(fp1, "\n");
	}
	fclose(fp1);
}

bool test_fstream()
{
	float** bufff = new float* [BUFFER_LEN];
	for (int i = 0; i < BUFFER_LEN; i++)
	{
		bufff[i] = new float[BUFFER_WIDTH];
		std::fill(bufff[i], bufff[i] + BUFFER_WIDTH, i + 0.1);
	}
	auto start = std::chrono::system_clock::now();
	std::ofstream fout("csvTest_fstream.csv", std::ios::out);
	if (!fout.is_open())
	{
		std::cout << "can not open" << std::endl;
		return false;
	}
	for (int i = 0; i < BUFFER_LEN; i++)
	{
		fout << bufff[i][0] << "," << bufff[i][1] << "," << bufff[i][2] << "," << bufff[i][3] << "," << bufff[i][4] << "," << bufff[i][5] << "," << bufff[i][6] << "," << bufff[i][7]
			<< "," << bufff[i][8] << "," << bufff[i][9] << "," << bufff[i][10] << "," << bufff[i][11] << "," << bufff[i][12] << "," << bufff[i][13] << "," << bufff[i][14] << "," << bufff[i][15]
			<< "," << bufff[i][16] << "," << bufff[i][17] << "," << bufff[i][18] << "," << bufff[i][19] << "," << bufff[i][20] << "," << bufff[i][21] << "," << bufff[i][22] << "," << bufff[i][23]
			<< "," << bufff[i][24] << "," << bufff[i][25] << "," << bufff[i][26] << "," << bufff[i][27] << "," << bufff[i][28] << "," << bufff[i][29] << "," << bufff[i][30] << "," << bufff[i][31] << "\n";

	}
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "花费了"
		<< double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << std::endl;
	fout.close();
	return true;
}

void getFiles(std::string path, std::vector<std::string>& files)
{
	intptr_t hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*.dat").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

int main()
{
	// 多线程存文件测试
	/*std::threadpool td(5);
	auto start = std::chrono::system_clock::now();
	for (int i = 0; i < BUFFER_LEN; i++)
	{
		std::fill(bufff[i], bufff[i] + BUFFER_WIDTH, i + 0.1);
	}
	std::future<void> ListenThread1 = td.commit(test_fprintf_array, bufff, 1);

	for (int i = 0; i < BUFFER_LEN; i++)
	{
		std::fill(bufff[i], bufff[i] + BUFFER_WIDTH, i + 0.2);
	}
	std::future<void> ListenThread2 = td.commit(test_fprintf_array, bufff, 2);

	for (int i = 0; i < BUFFER_LEN; i++)
	{
		std::fill(bufff[i], bufff[i] + BUFFER_WIDTH, i + 0.3);
	}
	std::future<void> ListenThread3 = td.commit(test_fprintf_array, bufff, 3);
	ListenThread1.get();
	ListenThread2.get();
	ListenThread3.get();
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "花费了"
		<< double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << std::endl;*/
	
	//std::string filename;
	//std::cout << "输入文件名: ";
	//char szbuf[128] = { 0 };
	//std::cin.getline(szbuf, 128);
	//std::cout << "\n正在转换......" << std::endl;
	//test_readDAT(szbuf);

	int board_num = 0;
	int save_time = 0;
	std::cout << "输入板子数: ";
	std::cin >> board_num;
	std::cout << "输入存文件时间(s): ";
	std::cin >> save_time;
	std::cout << "板子数为: " << board_num << "    " << "存文件时间为: " << save_time << "s" << std::endl;

	const char* filePath = ".";
	std::vector<std::string> files;
	getFiles(filePath, files);
	int size = files.size();
	std::cout << "共" << size << "个dat文件" << std::endl;
	for (int i = 0; i < size; i++)
	{
		std::cout << files[i].c_str() << "  正在转换......" << std::endl;
		test_readDAT(files[i].c_str(), board_num, save_time);
	}
	system("pause");
	return 0;
}