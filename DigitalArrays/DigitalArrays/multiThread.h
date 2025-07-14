#ifndef MULTITTHREAD_H
#define MULTITTHREAD_H

#include <QThread>
#include <qqueue.h>
#include <qpushbutton.h>
#include "fftwdll.h"
#include "IOCP_DLL.h"
#include "RingBuffer.h"
#include <QMutex>
#include <QMutexLocker>
#include <QObject>

#pragma comment(lib,"FFTW_DLL.lib")
#pragma comment(lib,"libfftw3-3.lib")
#pragma comment(lib,"libfftw3f-3.lib")
#pragma comment(lib,"libfftw3l-3.lib")
#pragma comment(lib,"IOCP0401_test.lib")

#define SAMPLE_RATE 50000                                                   // 采样率

#define QCUSTOMPLOT_READTIME_SHOW_NUM_1s 50000								// 实时数据绘制横坐标长度 也就是绘制1s的数据 50k
#define QCUSTOMPLOT_READTIME_SHOW_NUM_100ms 5000							// 实时数据绘制横坐标长度 也就是绘制100ms的数据 5k

#define QCUSTOMPLOT_READTIME_SHOW_NUM_100K 100000							// 实时数据绘制横坐标长度 也就是绘制1s的数据 100k
#define QCUSTOMPLOT_FFT_SHOW_NUM 2048										// fft绘制横坐标长度 也就是拿8*4096个点作fft 
#define QCUSTOMPLOT_DATA_ACCUMULATION_SHOW_NUM 2048					        // 数据累积
#define QCUSTOMPLOT_FLUSH_50K_COUNT 2048                                    // 来多少个数据刷新一次实时显示 50k  50000/2048 = 24.4帧
#define QCUSTOMPLOT_FLUSH_100K_COUNT 4096                                   // 来多少个数据刷新一次实时显示 100k 100000/4096 = 24.4帧
#define QCUSTOMPLOT_FLUSH_25K_COUNT 1024                                   // 来多少个数据刷新一次实时显示 100k 100000/4096 = 24.4帧

#define RINGBUFFER_LENGTH 2097152									        // ringbuffer的length 2的20次方 
#define CHANNEL 16                                                          // adc通道数

#define SAMPLE_CNT_TO_TIME_50K 0.00002                                      // 采样的点数转换为时间
#define SAMPLE_CNT_TO_TIME_25K 0.00004                                      // 采样的点数转换为时间
#define SAMPLE_CNT_TO_TIME_100K 0.00001                                     // 采样的点数转换为时间

#define CMD_SELFCHECK 2                                                     // 自检命令
#define CMD_BEGIN 3                                                         // 启动命令

//RingBuffer* _ringBuffer= new RingBuffer(RINGBUFFER_LENGTH, 288, QCUSTOMPLOT_FLUSH_50K_COUNT);

/*
* @brief IndicatorLight 实时数据按钮和数据保存按钮指示灯
*/

class IndicatorLight : public QPushButton
{
    Q_OBJECT
public:
    IndicatorLight(int indicatorLight_mode, QWidget* parent = nullptr) : QPushButton(parent), _indicatorLight_mode(indicatorLight_mode)
    {
        setCheckable(false);
        setFixedSize(28, 28);

        updateButtonStyle(_indicatorLight_mode, false);
    }

    void setState(bool state)
    {
        //setChecked(state);
        updateButtonStyle(_indicatorLight_mode, state);
    }

private:
    int _indicatorLight_mode;

    // indicatorLight_mode = 1表示存数据和开始显示
    // indicatorLight_mode = 0表示系统状态
    void updateButtonStyle(int indicatorLight_mode, bool state)
    {
        if (indicatorLight_mode)
        {
            if (state)
            {
                setStyleSheet("QPushButton { background-color: green; }");
                setText("ON");
            }
            else
            {
                setStyleSheet("QPushButton { background-color: red; }");
                setText("OFF");
            }
        }
        else
        {
            if (state)
            {
                setStyleSheet("QPushButton { background-color: green; }");
                setText("OK");
            }
            else
            {
                setStyleSheet("QPushButton { background-color: red; }");
                setText("NO");
            }
        }
    }
};

class GetDataThread : public QThread
{
    Q_OBJECT
public:
    explicit GetDataThread(uint32_t get_data_num, uint32_t boards_num, uint32_t sampleRate_GDThread, QObject* parent = nullptr);

    void stopThread();
    void beginThread();
    void emit_signal();
    
    RingBuffer* _ringBuffer;
    double* energy_sum;

protected:
    void run();

private:
    uint32_t _dataNum;
    uint32_t _boardsNum;
    uint32_t _channelNum;
    uint32_t _perTimeBytes;
    bool _isStop = true;
    uint32_t* _recBuf;
    uint32_t _sampleRate_GDThread;
signals:
    void trans_ringbuffer(void* lpParam, int boards_nums);
};

/*
* 从ringbuffer里边取数据，然后通过信号与槽发送给界面更新绘图
*/
class DoUpdateThread : public QThread
{
    Q_OBJECT
public:
    explicit DoUpdateThread(int xAxis, QObject* parent = nullptr);

    void stopThread();
    void beginThread();

    void realTimeDisplay(bool flag);
    void setTabWidgetIdx(int idx);

protected:
    void run();

private:
    bool _isStop = true;
    RingBuffer* _ringBuffer;
    double** RealTime_data;                                               // 通道数 * 每秒数据量(288*50000) 的一帧图像显示内存
    uint32_t _boardsNum;
    uint32_t energy_rec_cnt;

    double* FFT_keys;
    double* FFT_values;
    double* energy_accumulation;                                            // 显示一个板子的 16个adc通道(和1个485通道)的能谱图
    double* energy_accumulation_all;

    int FFT_board;                                                          // fft板子选择
    int FFT_channel;                                                        // fft通道选择

    int Energy_board;                                                       // 能谱图板子选择

    double* fft_dataBuffer;                                                 // 保存每次接收数据 用来做fft

    bool real_time_flag;                                                    // 控制是否实时显示

    int xAxis_count;
    int xAxis_count_temp;

    bool is_xAxis_changed;

    int sampleRate_DUThread;
    int fft_len;

    //double** RealTime_data_500;
    //double** RealTime_data_5000;
    double** RealTime_data_50000;

    QMutex mtx_RealTimeData;

    int tabWidgetIdx;
signals:
    void update_customPlot(double** real_time_data, double* FFT_keys, double* FFT_values, double* energy_accumulation, uint32_t length, bool is_changed_xAxis, uint32_t mode);


public slots:
    void do_trans_ringbuffer(void* lpParam, int boards_nums);
    void do_fft_board_changed(int value);
    void do_fft_channel_changed(int value);
    void do_energy_board_changed(int value);
    void do_xAxis_changed(int value);
};



#endif // MULTITTHREAD_H