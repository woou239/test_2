#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <typeinfo>
#include <QtWidgets/QMainWindow>
#include "ui_DigitalArrays.h"
#include "multiThread.h"
#include "IOCP_DLL.h"
#include <windows.h>
#include <signal.h>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
//#include <dbghelp.h>
//
//LONG ApplicationCrashHandler(EXCEPTION_POINTERS* pException);


class DigitalArrays : public QMainWindow
{
    Q_OBJECT

public:
    DigitalArrays(QWidget *parent = nullptr);
    ~DigitalArrays();
    void Main_plotInit();
    void getDiskInfo();
    void init_control_set();


private:
    Ui::DigitalArraysClass ui;

    QCustomPlot* customplot1;                                               // 实时图像
    QCustomPlot* customplot2;                                               // FFT图像
    QCustomPlot* customplot3;                                               // 通道能量累积

    // 磁盘容量统计
    double freeSize, totalSize;

    QVector<QCPGraphData>* mData_buffer_realtime;                           // 绘图直接操作内存，但是setData()直接替换数据

    QCPBars* subBars;                                                       // 柱状图        

    IndicatorLight* indicatorLight_realtime;                                // 实时数据指示灯
    IndicatorLight* indicatorLight_savefiles;                               // 存文件指示灯

    GetDataThread *GDThread;                                                // 线程
    DoUpdateThread* DUThread;

    uint32_t** RealTime_data;                                               // 288 * 50000 的一帧图像显示内存

    QCPGraph* Customplot1_graph[8];                                         // 8通道 数据采集 graph[i]

    char* Save_path;                                                        // 保存文件路径 传给init()
    std::string string_save_path;

    int Boards_nums;                                                        // 板子数量
    int Channel_nums;                                                       // 所有板子的通道数量

    QVector<double> ticks;                                                  // 能谱图横轴
    QVector<QString> labels;                                                // 能谱图labels
    QVector<QString> labels2;                                                // 能谱图labels
    QVector<double> y1;

    int** board_add_channel;                                                // 记录实时绘图的板子和通道

    QSpinBox* board_sbox[8];                                                // 板子spinbox 8
    QSpinBox* channel_sbox[8];                                              // 通道spinbox 8

    int xAxis_num;                                                          // 实时显示x轴范围

    bool realTime_display_flag;                                             // 实时显示标志

    int sampleRate;                                                         // 要发给fpga的采样率

    int saveFileNum;                                                        // 保存文件大小，传给init

    double **altitude_data;                                                 // 姿态传感器数据Roll,Pitch,Yaw

    double* press;                                                          // 压力传感器

    QString press_symbol[3];                                                // 压力数据单位 

    QLineEdit** altitudeLineEdit;                                           // 姿态数据 QLineEdit 数组
    QLineEdit** pressLineEdit;                                              // 压力数据 QLineEdit 数组

    std::vector<IndicatorLight*> system_state_group_indicatorLight;         // 系统状态的指示灯 2*11个 从0到21

    QLabel* _labInfo;
    QMutex mtx_board_channal_select;

    int tabWidget_idx;

    double sample_num2time;

    int fft_len_adaptive_sample;
protected:
    void closeEvent(QCloseEvent* event);

private slots:
    //void pButtonClicked();

    void pButton_get_Channels_Clicked();

    void pButton_get_SavePath_Clicked();
    void comboBox_saveSize();
    void pButtton_test();
    void do_valueChanged(qint64 data1, qint64 data2);
    void pButton_begin();
    void pButton_save();
    //void pButton_stop();
    void pButton_connectDevice();
    void sBox_num_of_boards_valueChanged(int);
    void sBox_fft_board_valueChanged(int);
    void sBox_fft_channel_valueChanged(int);
    void sBox_energy_board_valueChanged(int);
    void sBox_yAxis_valueChanged(double);
    void comboBox_sampleRateChanged(int);
    void sBox_yAxis_fft_valueChanged(double);
    void sBox_yAxis_energy_valueChanged(double);
    void pButton_selfcheck();
    void pButton_systemEnable();

    void checkBox_graph1(bool isVisable);
    void checkBox_graph2(bool isVisable);
    void checkBox_graph3(bool isVisable);
    void checkBox_graph4(bool isVisable);
    void checkBox_graph5(bool isVisable);
    void checkBox_graph6(bool isVisable);
    void checkBox_graph7(bool isVisable);
    void checkBox_graph8(bool isVisable);

    void do_update_customPlot(double** realtime_values, double* FFT_keys, double* FFT_values, double* energy_accumulation, uint32_t length, bool is_changed_xAxis, uint32_t mode);
    //void do_update_customPlot_energy(double* energy_accumulation);
    void do_spinbox_value_changed(int value);

    void comboBox_xAxis(int idx);

    void tabWidget_IdxChanged(int idx);

public slots:


signals:
    void newPoint(qint64 data1, qint64 data2);
    void fft_board_changed(int value);
    void fft_channel_changed(int value);
    void energy_board_changed(int value);
    void xAxis_changed(int value);
    void tabwidget_init_idx(int num);
};

class StatusInfo {

};

