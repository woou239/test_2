#include "matplotlibcpp.h"
#include <vector>
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <io.h>

#define SAMPLE_RATE 50000
#define REF_LISTENER (1.1111*0.000000488292)

namespace plt = matplotlibcpp;

int trans2Idx(int board, int channel)
{
	return (board - 1) * 18 + channel - 1;
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

void plot_png(std::string FileName, int board_num, int save_num, std::vector<int> &idx, int size)
{
	int length = size >> 1;

	std::vector<double> y;

	int bufferLen = save_num * SAMPLE_RATE;
	int bufferWidth = board_num * 18;

	uint32_t** bufff = new uint32_t * [bufferLen];
	for (int i = 0; i < bufferLen; i++)
	{
		bufff[i] = new uint32_t[bufferWidth];
	}

	FILE* fp = fopen(FileName.c_str(), "rb");

	for (int i = 0; i < bufferLen; i++)
	{
		fread(bufff[i], sizeof(uint32_t), bufferWidth, fp);
	}
	fclose(fp);


	for (int j = 0; j < bufferLen / 50000; j++)
	{
		std::string png_name = FileName;
		png_name = png_name.substr(0, png_name.length() - 4);
		//png_name = "b-" + std::to_string(idx[k]) + " c-" + std::to_string(idx[k + length]) + " " + png_name + " " + std::to_string(j * 50000) + "-" + std::to_string((j + 1) * 50000) + ".jpg";
		png_name = png_name + " " + std::to_string(j * 50000) + "-" + std::to_string((j + 1) * 50000) + ".jpg";
		std::cout << png_name << std::endl;
		plt::figure_size(2000, 600);
		for (int k = 0; k < length; k++)
		{
			for (int i = j * 50000; i < (j + 1) * 50000; ++i)
			{
				y.push_back(REF_LISTENER * TransForm(bufff[i][trans2Idx(idx[k], idx[k + length])]));
			}
			plt::plot(y, { {"label", "b:" + std::to_string(idx[k]) + " c:" + std::to_string(idx[k + length])} });
			plt::legend();
			y.clear();
		}
		plt::save(png_name.c_str());
		plt::close();
		y.clear();
	}

}

int main() {
	int board_num = 0;
	int save_time = 0;
	int temp;
	std::vector<int> board;
	std::string TXT_file_name = "board_channel_input.txt";
	std::ifstream TXT_file;
	TXT_file.open(TXT_file_name.c_str(), std::ios::in | std::ios::binary); 

	TXT_file >> board_num;
	TXT_file >> save_time;
	while (TXT_file >> temp)
	{
		board.push_back(temp);
	}


	std::cout << "板子数为: " << board_num << "    " << "存文件时间为: " << save_time << "s" << std::endl;

	const char* filePath = ".";
	std::vector<std::string> files;
	getFiles(filePath, files);
	int size = files.size();
	std::cout << "共" << size << "个dat文件" << std::endl;

	for (int i = 0; i < size; i++)
	{
		std::cout << files[i].c_str() << "  正在转换png......" << std::endl;
		plot_png(files[i].c_str(), board_num, save_time, board, board.size());
	}

	//std::vector<int> x = { 4,5,6,7 };
	//std::vector<int> y = { 1,2,3,4 };
	//std::vector<int> y1 = { 4,5,6,7 };
	//plt::plot(x,y, { {"label", "111"} });
	//plt::legend();
	//plt::plot(x,y1);
	////plt::show();
	//plt::save("b1 c8 1  2024-9-5 11-28-48 0-50000.jpg");

	system("pause");
	//plt::savefig("minimal.pdf");
}