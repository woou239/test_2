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

    QCustomPlot* customplot1;                                               // ʵʱͼ��
    QCustomPlot* customplot2;                                               // FFTͼ��
    QCustomPlot* customplot3;                                               // ͨ�������ۻ�

    // ��������ͳ��
    double freeSize, totalSize;

    QVector<QCPGraphData>* mData_buffer_realtime;                           // ��ͼֱ�Ӳ����ڴ棬����setData()ֱ���滻����

    QCPBars* subBars;                                                       // ��״ͼ        

    IndicatorLight* indicatorLight_realtime;                                // ʵʱ����ָʾ��
    IndicatorLight* indicatorLight_savefiles;                               // ���ļ�ָʾ��

    GetDataThread *GDThread;                                                // �߳�
    DoUpdateThread* DUThread;

    uint32_t** RealTime_data;                                               // 288 * 50000 ��һ֡ͼ����ʾ�ڴ�

    QCPGraph* Customplot1_graph[8];                                         // 8ͨ�� ���ݲɼ� graph[i]

    char* Save_path;                                                        // �����ļ�·�� ����init()
    std::string string_save_path;

    int Boards_nums;                                                        // ��������
    int Channel_nums;                                                       // ���а��ӵ�ͨ������

    QVector<double> ticks;                                                  // ����ͼ����
    QVector<QString> labels;                                                // ����ͼlabels
    QVector<QString> labels2;                                                // ����ͼlabels
    QVector<double> y1;

    int** board_add_channel;                                                // ��¼ʵʱ��ͼ�İ��Ӻ�ͨ��

    QSpinBox* board_sbox[8];                                                // ����spinbox 8
    QSpinBox* channel_sbox[8];                                              // ͨ��spinbox 8

    int xAxis_num;                                                          // ʵʱ��ʾx�᷶Χ

    bool realTime_display_flag;                                             // ʵʱ��ʾ��־

    int sampleRate;                                                         // Ҫ����fpga�Ĳ�����

    int saveFileNum;                                                        // �����ļ���С������init

    double **altitude_data;                                                 // ��̬����������Roll,Pitch,Yaw

    double* press;                                                          // ѹ��������

    QString press_symbol[3];                                                // ѹ�����ݵ�λ 

    QLineEdit** altitudeLineEdit;                                           // ��̬���� QLineEdit ����
    QLineEdit** pressLineEdit;                                              // ѹ������ QLineEdit ����

    std::vector<IndicatorLight*> system_state_group_indicatorLight;         // ϵͳ״̬��ָʾ�� 2*11�� ��0��21

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

