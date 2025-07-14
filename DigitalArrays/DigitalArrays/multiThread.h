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

#define SAMPLE_RATE 50000                                                   // ������

#define QCUSTOMPLOT_READTIME_SHOW_NUM_1s 50000								// ʵʱ���ݻ��ƺ����곤�� Ҳ���ǻ���1s������ 50k
#define QCUSTOMPLOT_READTIME_SHOW_NUM_100ms 5000							// ʵʱ���ݻ��ƺ����곤�� Ҳ���ǻ���100ms������ 5k

#define QCUSTOMPLOT_READTIME_SHOW_NUM_100K 100000							// ʵʱ���ݻ��ƺ����곤�� Ҳ���ǻ���1s������ 100k
#define QCUSTOMPLOT_FFT_SHOW_NUM 2048										// fft���ƺ����곤�� Ҳ������8*4096������fft 
#define QCUSTOMPLOT_DATA_ACCUMULATION_SHOW_NUM 2048					        // �����ۻ�
#define QCUSTOMPLOT_FLUSH_50K_COUNT 2048                                    // �����ٸ�����ˢ��һ��ʵʱ��ʾ 50k  50000/2048 = 24.4֡
#define QCUSTOMPLOT_FLUSH_100K_COUNT 4096                                   // �����ٸ�����ˢ��һ��ʵʱ��ʾ 100k 100000/4096 = 24.4֡
#define QCUSTOMPLOT_FLUSH_25K_COUNT 1024                                   // �����ٸ�����ˢ��һ��ʵʱ��ʾ 100k 100000/4096 = 24.4֡

#define RINGBUFFER_LENGTH 2097152									        // ringbuffer��length 2��20�η� 
#define CHANNEL 16                                                          // adcͨ����

#define SAMPLE_CNT_TO_TIME_50K 0.00002                                      // �����ĵ���ת��Ϊʱ��
#define SAMPLE_CNT_TO_TIME_25K 0.00004                                      // �����ĵ���ת��Ϊʱ��
#define SAMPLE_CNT_TO_TIME_100K 0.00001                                     // �����ĵ���ת��Ϊʱ��

#define CMD_SELFCHECK 2                                                     // �Լ�����
#define CMD_BEGIN 3                                                         // ��������

//RingBuffer* _ringBuffer= new RingBuffer(RINGBUFFER_LENGTH, 288, QCUSTOMPLOT_FLUSH_50K_COUNT);

/*
* @brief IndicatorLight ʵʱ���ݰ�ť�����ݱ��水ťָʾ��
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

    // indicatorLight_mode = 1��ʾ�����ݺͿ�ʼ��ʾ
    // indicatorLight_mode = 0��ʾϵͳ״̬
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
* ��ringbuffer���ȡ���ݣ�Ȼ��ͨ���ź���۷��͸�������»�ͼ
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
    double** RealTime_data;                                               // ͨ���� * ÿ��������(288*50000) ��һ֡ͼ����ʾ�ڴ�
    uint32_t _boardsNum;
    uint32_t energy_rec_cnt;

    double* FFT_keys;
    double* FFT_values;
    double* energy_accumulation;                                            // ��ʾһ�����ӵ� 16��adcͨ��(��1��485ͨ��)������ͼ
    double* energy_accumulation_all;

    int FFT_board;                                                          // fft����ѡ��
    int FFT_channel;                                                        // fftͨ��ѡ��

    int Energy_board;                                                       // ����ͼ����ѡ��

    double* fft_dataBuffer;                                                 // ����ÿ�ν������� ������fft

    bool real_time_flag;                                                    // �����Ƿ�ʵʱ��ʾ

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