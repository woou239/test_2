#include "multiThread.h"

// param1:每次获取的数据量 param2:板子数量
GetDataThread::GetDataThread(uint32_t get_data_num, uint32_t boards_num, uint32_t sampleRate_GDThread, QObject* parent)
	: QThread{ parent }, _dataNum(get_data_num)
{
	qRegisterMetaType<QVariant>("QVariant");
	_isStop = true;

	_boardsNum = boards_num;
	_channelNum = boards_num * 18;
	_perTimeBytes = boards_num * 18 * 4;
	_recBuf = new uint32_t[get_data_num * _channelNum];

	energy_sum = new double[_channelNum];
	_sampleRate_GDThread = sampleRate_GDThread;
	int flush_cnt = 0;
	switch (_sampleRate_GDThread)
	{
	case 50000:flush_cnt = QCUSTOMPLOT_FLUSH_50K_COUNT; break;
	case 25000:flush_cnt = QCUSTOMPLOT_FLUSH_25K_COUNT; break;
	case 100000:flush_cnt = QCUSTOMPLOT_FLUSH_100K_COUNT; break;
	default:flush_cnt = QCUSTOMPLOT_FLUSH_50K_COUNT;
	}

	_ringBuffer = new RingBuffer(RINGBUFFER_LENGTH, _channelNum, flush_cnt);

}

void GetDataThread::stopThread()
{
	_isStop = true;
}

void GetDataThread::beginThread()
{
	_isStop = false;
}

void GetDataThread::emit_signal()
{
	emit(trans_ringbuffer((void*)_ringBuffer, _boardsNum));
}


/*
* 不使用 recfifo, 感觉每次get_data()之间花费的时间太多，且emit出去, 画图有问题
* 因此考虑换用两个线程，一个一直get_data()存到 recfifo
* 另外一个线程把数据取出来，到了一定的数据了再取出来给qcustomplot画图
* 
* 后续不用recfifo，用着不习惯 改用ringbuffer，来充当fifo缓冲区
*/
//void GetDataThread::run()
//{
//	while (1)
//	{
//		while (!_isStop)
//		{
//			QVariant dataVar;
//			uint32_t temp_data = 0;
//			if (get_data(_recBuf, _dataNum))
//			{
//				/*memcpy(_realTimeBuf + _realTimeCount * _dataNum, _recBuf, _dataNum);*/
//				for (int i = 0; i < _dataNum; i++)
//				{
//					for (int j = 0; j < _channelNum; j++)
//					{
//						memcpy(&temp_data, _recBuf + i * _perTimeBytes + j * 4, 4);
//						temp_data = temp_data & 0x0000ff;
//						_realTimeData[j].append(temp_data);
//					}
//				}
//				//memcpy(_fftBuf + _fftCount * _dataNum, _recBuf, _dataNum);
//				//memcpy(_dataAccumulationBuf + _dataAcumulationCount * _dataNum, _recBuf, _dataNum);
//				_realTimeCount++;
//				_fftCount++;
//				_dataAcumulationCount++;
//				if (_realTimeCount * _dataNum == QCUSTOMPLOT_FLUSH_50K_COUNT)
//				{
//					_realTimeCount = 0;
//					dataVar.setValue(_realTimeData[0]);
//					emit(update_customPlot(dataVar, nullptr, nullptr, nullptr, QCUSTOMPLOT_FLUSH_50K_COUNT, 0));
//					for (int j = 0; j < _channelNum; j++)
//					{
//						_realTimeData[j].clear();
//					}
//				}
//				if (_fftCount * _dataNum == QCUSTOMPLOT_FFT_SHOW_NUM)
//				{
//					/*fftw(QCUSTOMPLOT_FFT_SHOW_NUM, QCUSTOMPLOT_READTIME_SHOW_NUM_50K, _fftBuf, FFT_keys, FFT_values);*/
//					_fftCount = 0;
//					/*emit(update_customPlot(nullptr, FFT_keys, FFT_values, nullptr, QCUSTOMPLOT_FFT_SHOW_NUM, 1));*/
//				}
//				if (_dataAcumulationCount * _dataNum == QCUSTOMPLOT_DATA_ACCUMULATION_SHOW_NUM)
//				{
//					_dataAcumulationCount = 0;
//					//emit
//				}
//			}
//		}
//	}
//}

void GetDataThread::run()
{
	while (1)
	{
		while (!_isStop)
		{
			QVariant dataVar;
			uint32_t temp_data = 0;
			if (get_data(_recBuf, _dataNum))
			{
				//qDebug() << Qt::hex << _recBuf[0];
				for (int i = 0; i < _dataNum; i++)
				{
					// 改进计算能谱图
					//for (int j = 0; j < _channelNum; j++)
					//{
					//	energy_sum[j]+= _dataNum[]
					//}
					_ringBuffer->adds_with_sync_code(_recBuf + i * _channelNum);
				}
			}
			//if (rec_cnt >= QCUSTOMPLOT_FLUSH_50K_COUNT)
			//{
			//	rec_cnt = rec_cnt - QCUSTOMPLOT_FLUSH_50K_COUNT;
			//	emit(update_customPlot(dataVar, nullptr, nullptr, nullptr, QCUSTOMPLOT_FLUSH_50K_COUNT, 0, this));
			//}

		}
	}
}



DoUpdateThread::DoUpdateThread(int xAxis, QObject* parent)
	: QThread{ parent }
{
	xAxis_count = xAxis;
	sampleRate_DUThread = xAxis_count * 10;
	//QCUSTOMPLOT_FLUSH_50K_COUNT 每次刷新的点，也就是计算的点
	switch (sampleRate_DUThread)
	{
	case 50000:fft_len = QCUSTOMPLOT_FLUSH_50K_COUNT; break;
	case 25000:fft_len = QCUSTOMPLOT_FLUSH_25K_COUNT; break;
	case 100000:fft_len = QCUSTOMPLOT_FLUSH_100K_COUNT; break;
	default:fft_len = QCUSTOMPLOT_FLUSH_50K_COUNT;
	}

	fft_dataBuffer = new double[fft_len];
	FFT_keys = new double[fft_len];
	FFT_values = new double[fft_len];
	energy_accumulation = new double[CHANNEL];
	for (int i = 0; i < CHANNEL; i++)
	{
		energy_accumulation[i] = 0;
	}

	energy_accumulation_all = new double[CHANNEL * 11];
	for (int i = 0; i < CHANNEL * 11; i++)
	{
		energy_accumulation_all[i] = 0;
	}

	FFT_board = 1;
	FFT_channel = 1;
	Energy_board = 1;
	real_time_flag = false;

	is_xAxis_changed = false;
	tabWidgetIdx = 0;
	
}

void DoUpdateThread::stopThread()
{
	_isStop = true;
}

void DoUpdateThread::beginThread()
{
	_isStop = false;
}

void DoUpdateThread::run()
{
	while (1)
	{
		while (!_isStop)
		{
			if (_ringBuffer->removes_with_sync_code(RealTime_data, xAxis_count))
			{
				if (real_time_flag)
				{
					if (tabWidgetIdx == 1)
					{
						// 计算fft
						memcpy(fft_dataBuffer, RealTime_data[trans2Idx(FFT_board, FFT_channel)], fft_len << 3);
						fftw(fft_len, sampleRate_DUThread, fft_dataBuffer, FFT_keys, FFT_values);
						update_customPlot(RealTime_data, FFT_keys, FFT_values, energy_accumulation_all, xAxis_count, is_xAxis_changed, 1);
						if (is_xAxis_changed == true)
						{
							is_xAxis_changed = false;
						}
					}
					else if (tabWidgetIdx == 2)
					{
#ifdef NODE_18
						int boardsNum_temp = 2;
#endif
#ifdef NODE_11_50K_SAMPLE
						int boardsNum_temp = 10;
#elif NODE_4_ADAPTIVE_SAMPLE
						int boardsNum_temp = 4;
#endif
						for (int i = 0; i < 16 * boardsNum_temp; i++)
						{
#ifdef NODE_18
							if (i == 13 || i == 16 || i == 18 || i == 19 || i == 20 || i == 22 || i == 23 ||
								i == 24 || i == 25 || i == 26 || i == 28 || i == 29 || i == 30 || i == 31)
							{
								energy_accumulation_all[i] = 0;
								continue;
							}
#endif
							for (int j = 0; j < fft_len; j++)
							{
								energy_accumulation_all[i] += fabs(RealTime_data[trans2Idx((i / 16 + 1), (i % 16 + 1))][j]);
							}
						}
						update_customPlot(RealTime_data, FFT_keys, FFT_values, energy_accumulation_all, xAxis_count, is_xAxis_changed, 2);
					}
					else
					{

					}
				} 
					
			}
			

		}
	}
}

void DoUpdateThread::do_trans_ringbuffer(void* lpParam, int boards_nums)
{
	_ringBuffer = (RingBuffer*)lpParam;
	_boardsNum = boards_nums;

	RealTime_data_50000 = new double* [_boardsNum * 18];
	for (int i = 0; i < _boardsNum * 18; i++)
	{
		RealTime_data_50000[i] = new double[sampleRate_DUThread];
	}
	RealTime_data = RealTime_data_50000;
	//qDebug() << "RealTime_data address: " << RealTime_data;
}

void DoUpdateThread::do_fft_board_changed(int value)
{
	FFT_board = value;
	//qDebug() << "FFT_board" << FFT_board;
}

void DoUpdateThread::do_fft_channel_changed(int value)
{
	FFT_channel = value;
	//qDebug() << "FFT_channel" << FFT_channel;
}

void DoUpdateThread::do_energy_board_changed(int value)
{
	Energy_board = value;
	//qDebug() << "Energy_board" << Energy_board;
}

void DoUpdateThread::realTimeDisplay(bool flag)
{
	real_time_flag = flag;
}

void DoUpdateThread::do_xAxis_changed(int value)
{
	is_xAxis_changed = true;
	xAxis_count = value;
}

void DoUpdateThread::setTabWidgetIdx(int idx)
{
	tabWidgetIdx = idx;
}