#include "DigitalArrays.h"
#include <qdir.h>
#include <QFileDialog>
#include "qcustomplot.h"
#include <QStorageInfo>
#include <qmath.h>
#include <Windows.h>
#include <QCloseEvent>

#define DEFAULT_BOARD_NUMS 16


DigitalArrays::DigitalArrays(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    //setCentralWidget(ui.tabWidget);
    fft_len_adaptive_sample = QCUSTOMPLOT_FLUSH_50K_COUNT;
    sample_num2time = SAMPLE_CNT_TO_TIME_50K;
    tabWidget_idx = 0;

    //标题
#ifdef NODE_11_50K_SAMPLE
    ui.label_6->setText(QString::fromLocal8Bit("161元拖曳阵数字采集系统"));
#elif NODE_4_ADAPTIVE_SAMPLE
    ui.label_6->setText(QString::fromLocal8Bit("4节点垂直阵数字采集系统"));
#else
    ui.label_6->setText(QString::fromLocal8Bit("xxx"));
#endif

    _labInfo = new QLabel();
    ui.statusBar->addWidget(_labInfo);

    // 11节点需要4个姿态 4个压力，4节点需要2个倾角 2个压力 对应的DigitallArrays.ui也需要改 后期可以代码生成控件，不需要改对应的ui
#ifdef NODE_11_50K_SAMPLE
    int need_count_of_485 = 4;
#elif NODE_4_ADAPTIVE_SAMPLE
    int need_count_of_485 = 2;
#else
    int need_count_of_485 = 0;
#endif

    /******************************0223 此处由ui生成lEdit_altitude1 - lEdit_altitude4 还需要重新归为数组 改4节点需注意******************************/ 

    altitudeLineEdit = new QLineEdit * [need_count_of_485];
    altitudeLineEdit[0] = ui.lEdit_altitude1;
    altitudeLineEdit[1] = ui.lEdit_altitude2;
    altitudeLineEdit[2] = ui.lEdit_altitude3;
    altitudeLineEdit[3] = ui.lEdit_altitude4;
 /*   altitudeLineEdit[4] = ui.lEdit_altitude5;*/

    pressLineEdit = new QLineEdit * [need_count_of_485];
    pressLineEdit[0] = ui.lEdit_pressure1;
    pressLineEdit[1] = ui.lEdit_pressure2;
    pressLineEdit[2] = ui.lEdit_pressure3;
    pressLineEdit[3] = ui.lEdit_pressure4;
    //pressLineEdit[4] = ui.lEdit_pressure5;

    /******************************0223 此处由ui生成lEdit_altitude1 - lEdit_altitude4 还需要重新归为数组 改4节点需注意******************************/
    altitude_data = new double* [need_count_of_485];
    press = new double[need_count_of_485];
    for (int i = 0; i < need_count_of_485; i++)
    {
        altitude_data[i] = new double[3];
    }

    /* ******************************************************     模拟的姿态压力数据     ************************************************************* */
    for (int i = 0; i < need_count_of_485; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            altitude_data[i][j] = 0;
        }
        press[i] = 0;
    }

    /* *********************************************************************************************************************************************** */

    press_symbol[0] = "MPa";
    press_symbol[1] = "KPa";
    press_symbol[2] = "Pa";

    // 姿态压力数据显示
    for (int i = 0; i < need_count_of_485; i++)
    {
        QString str1 = QString("roll:%1  pitch:%2  yaw:%3  ").arg(QString::number(altitude_data[i][0], 'f', 2)).arg(QString::number(altitude_data[i][1], 'f', 2)).arg(QString::number(altitude_data[i][2], 'f', 2));
        QString str2 = QString("press:%1 %2").arg(press[i]).arg(press_symbol[1]);
        altitudeLineEdit[i]->setText(str1);
        pressLineEdit[i]->setText(str2);
    }

    // 默认 存文件时间10s 采样率50k 实时显示图像横轴0.1s->5000个点 
    saveFileNum = 10;
    sampleRate = 50000;
    xAxis_num = 5000;
    realTime_display_flag = false;

#ifdef NODE_18
    Boards_nums = 2;
#endif
#ifdef NODE_11_50K_SAMPLE
    Boards_nums = 11;
#elif
    Boards_nums = 4;
#endif
    ui.spinBox_num_of_boards->setValue(Boards_nums);
    board_add_channel = new int* [8];
    for (int i = 0; i < 8; i++)
    {
        board_add_channel[i] = new int[2];
    }
    for (int i = 0; i < 8; i++)
    {
        board_add_channel[i][0] = 1;
        board_add_channel[i][1] = i + 1;
    }

    QSpinBox* board_spinbox[8] = { ui.spinBox_board1, ui.spinBox_board2, ui.spinBox_board3, ui.spinBox_board4, ui.spinBox_board5, ui.spinBox_board6, ui.spinBox_board7, ui.spinBox_board8 };
    QSpinBox* channel_spinbox[8] = { ui.spinBox_channel1, ui.spinBox_channel2, ui.spinBox_channel3, ui.spinBox_channel4, ui.spinBox_channel5, ui.spinBox_channel6, ui.spinBox_channel7, ui.spinBox_channel8 };

    for (int i = 0; i < 8; i++)
    {
        board_sbox[i] = board_spinbox[i];
        channel_sbox[i] = channel_spinbox[i];
        connect(board_spinbox[i], SIGNAL(valueChanged(int)), this, SLOT(do_spinbox_value_changed(int)));
        connect(channel_spinbox[i], SIGNAL(valueChanged(int)), this, SLOT(do_spinbox_value_changed(int)));
    }

    customplot1 = ui.customPlot_realtime;
    customplot2 = ui.customPlot_fft;
    customplot3 = ui.customPlot_accumulate;

    /*
    DUThread = new DoUpdateThread(xAxis_num);

    // 更新绘图
    connect(DUThread, &DoUpdateThread::update_customPlot, this, &DigitalArrays::do_update_customPlot);

    connect(this, &DigitalArrays::fft_board_changed, DUThread, &DoUpdateThread::do_fft_board_changed);
    connect(this, &DigitalArrays::fft_channel_changed, DUThread, &DoUpdateThread::do_fft_channel_changed);
    connect(this, &DigitalArrays::energy_board_changed, DUThread, &DoUpdateThread::do_energy_board_changed);
    connect(this, &DigitalArrays::xAxis_changed, DUThread, &DoUpdateThread::do_xAxis_changed);
    */
    
    // 状态栏


    // 实时数据 fft 能量累积 图的初始化
    Main_plotInit();
    init_control_set();

    // 磁盘容量显示
    freeSize = 0;
    totalSize = 0;

    // check_box数组


    // 开始、保存指示灯
    indicatorLight_realtime = new IndicatorLight(1);
    ui.horizontalLayout_11->addWidget(indicatorLight_realtime);
    indicatorLight_savefiles = new IndicatorLight(1);
    ui.horizontalLayout_12->addWidget(indicatorLight_savefiles);

    for (int i = 0; i < 12 * 2; i++)
    {
        system_state_group_indicatorLight.push_back(new IndicatorLight(0));
    }

    for (int i = 0; i < 11; i++)
    {
        ui.gridLayout_2->addWidget(system_state_group_indicatorLight[i * 2], i, 1);
        ui.gridLayout_2->addWidget(system_state_group_indicatorLight[i * 2 + 1], i, 2);
    }

    // 进度条显示
    connect(this, SIGNAL(newPoint(qint64, qint64)), this, SLOT(do_valueChanged(qint64, qint64)));
    
}

DigitalArrays::~DigitalArrays()
{
    //delete &ui;
}

void DigitalArrays::closeEvent(QCloseEvent* event)
{
    if (GDThread != nullptr)
    {
        if (GDThread->isRunning())
        {
            GDThread->terminate(); //强制终止线程
            GDThread->wait(); //等待线程结束
        }
    }
    if (DUThread->isRunning())
    {
        DUThread->terminate(); //强制终止线程
        DUThread->wait(); //等待线程结束
    }
    uninitialize();
    event->accept();
}

void DigitalArrays::init_control_set()
{
    // 使某些控件失效
    ui.pButton_selfcheck->setEnabled(false);
    ui.pButton_systembegin->setEnabled(false);
    ui.pButton_connect->setEnabled(false);
    ui.pButton_begin->setEnabled(false);
    ui.pButton_save->setEnabled(false);
    //ui.pButton_stop->setEnabled(false);
    ui.pButton_getStroage->setEnabled(false);
    ui.spinBox_fft_board->setEnabled(false);
    ui.spinBox_fft_channel->setEnabled(false);
    //ui.spinBox_energy_board->setEnabled(false);
    ui.cBox_xAxis->setEnabled(false);

    ui.graph1->setEnabled(false);
    ui.graph2->setEnabled(false);
    ui.graph3->setEnabled(false);
    ui.graph4->setEnabled(false);
    ui.graph5->setEnabled(false);
    ui.graph6->setEnabled(false);
    ui.graph7->setEnabled(false);
    ui.graph8->setEnabled(false);
}


void DigitalArrays::Main_plotInit()
{
    QColor q[8] = { QColor(0, 0, 0),QColor(255, 0, 255),QColor(255, 97, 0),QColor(65, 105, 225),
        QColor(0, 255, 0) ,QColor(168, 3, 38),QColor(160, 32, 240),QColor(94, 38, 18) };
    customplot1->legend->setVisible(true);
    customplot1->legend->setFont(QFont("Helvetica", 12));

    QPen pen;
    pen.setWidth(1);

    for (int i = 0; i < 8; i++)
    {
        Customplot1_graph[i] = customplot1->addGraph(); // blue line
        customplot1->graph(i)->setAdaptiveSampling(true);
        pen.setColor(q[i]);
        customplot1->graph(i)->setPen(pen);
    }

    //customplot1->graph(0)->setName("real-time graph");

    customplot1->xAxis2->setVisible(true);
    customplot1->xAxis2->setTickLabels(false);
    customplot1->yAxis2->setVisible(true);
    customplot1->yAxis2->setTickLabels(false);
    //customplot1->xAxis->setLabel("time(ms)");
    customplot1->yAxis->setLabel("voltage(V)");

    customplot1->xAxis->setRange(0, xAxis_num * SAMPLE_CNT_TO_TIME_50K, Qt::AlignLeft);
    customplot1->yAxis->setRange(-5, 5);
    //customplot1->yAxis->setRange(-1, 0x100);


    connect(customplot1->xAxis, SIGNAL(rangeChanged(QCPRange)), customplot1->xAxis2, SLOT(setRange(QCPRange)));
    connect(customplot1->yAxis, SIGNAL(rangeChanged(QCPRange)), customplot1->yAxis2, SLOT(setRange(QCPRange)));

    // 点的样式和隔多少点绘制
    /*customplot1->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);*/
    /*customplot1->graph(0)->setScatterSkip(500);*/


    // 可移动图像
    //customplot1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    /*customplot1->graph(0)->rescaleAxes();*/

    // 因为需要通过采样率来确定显示的点数，因此不能最开始就把点画好，等点击connect时再画
    //QVector<QCPGraphData> plotData(xAxis_num);
    //for (size_t j = 0; j < xAxis_num; j++)
    //{
    //    plotData[j].key = j * COUNT50000_TIME1;
    //    plotData[j].value = 0;
    //}

    //QSharedPointer<QCPGraphDataContainer> dataContainer[8];
    //for (int i = 0; i < 8; i++)
    //{
    //    dataContainer[i] = customplot1->graph(i)->data();
    //    dataContainer[i]->set(plotData, true);
    //}

    //mData_buffer_realtime = customplot1->graph(0)->data()->coreData();

    //for (int i = 0; i < 8; i++)
    //{
    //    Customplot1_graph[i]->setVisible(false);
    //}
    //Customplot1_graph[0]->setVisible(true);

    // fft图像初始化
    QCPGraph* fft_graph = customplot2->addGraph();
    customplot2->graph(0)->setAdaptiveSampling(true);
    fft_graph->setLineStyle(QCPGraph::lsLine);
    fft_graph->setPen(QPen(QColor("#FFA100"), 1));
    fft_graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 6));
    customplot2->xAxis->setRange(0, QCUSTOMPLOT_FFT_SHOW_NUM, Qt::AlignLeft);
    customplot2->yAxis->setRange(-190, 0.1);

    // 能谱图初始化
#ifdef NODE_18
    
    QVector<QString> str_up = { QString("A11"), QString("A9"), QString("A10"), QString("A12"), QString("A14"), QString("A16"), QString("A15"), QString("A13"),
        QString("A4"), QString("A1"), QString("A3"), QString("A2"), QString("A5"), QString(" "), QString("A7"), QString("A6"),
        QString(" "),QString("A8"), QString(" "), QString(" "), QString(" "), QString("A17"), QString(" "), QString(" "),
        QString(" "), QString(" "), QString(" "), QString("A18"), QString(" "), QString(" "), QString(" "), QString(" ")
        };
    
    for (int i = 0; i < 2 * 16; i++)
    {
        ticks << i + 1;
        QString str = QString("B%1C%2").arg(QString::number(i / 16 + 1)).arg(QString::number(i % 16 + 1));
        labels << str;
        labels2 << str_up[i];
    }

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    customplot3->xAxis->setTicker(textTicker);
    customplot3->xAxis->setTickLength(0, 1);
    customplot3->xAxis->ticker()->setTickCount(2 * 16 + 1);
    //customplot3->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);//可读性优于设置
    customplot3->xAxis->setRange(0, 33);
    //customplot3->xAxis->setTickLabelRotation(60);

    QSharedPointer<QCPAxisTickerText> textTicker2(new QCPAxisTickerText);
    textTicker2->addTicks(ticks, labels2);
    customplot3->xAxis2->setTicker(textTicker2);
    customplot3->xAxis2->setTickLength(0, 1);
    customplot3->xAxis2->ticker()->setTickCount(2 * 16 + 1);
    //customplot3->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);//可读性优于设置
    customplot3->xAxis2->setRange(0, 33);
    //customplot3->xAxis2->setTickLabelRotation(60);
    customplot3->xAxis2->setVisible(true);

#endif

#ifdef NODE_11_50K_SAMPLE
    for (int i = 0; i < 10 * 16; i++)
    {
        ticks << i + 1;
        QString str = QString("B%1C%2").arg(QString::number(i / 16 + 1)).arg(QString::number(i % 16 + 1));
        labels << str;
        QString str2 = QString("%1").arg(QString::number(i + 1));
        labels2 << str2;
    }
    

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    customplot3->xAxis->setTicker(textTicker);
    customplot3->xAxis->setTickLength(0, 1);
    customplot3->xAxis->ticker()->setTickCount(10 * 16 + 1);
    //customplot3->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);//可读性优于设置
    customplot3->xAxis->setRange(0, 161);
    customplot3->xAxis->setTickLabelRotation(60);

    QSharedPointer<QCPAxisTickerText> textTicker2(new QCPAxisTickerText);
    textTicker2->addTicks(ticks, labels2);
    customplot3->xAxis2->setTicker(textTicker2);
    customplot3->xAxis2->setTickLength(0, 1);
    customplot3->xAxis2->ticker()->setTickCount(10 * 16 + 1);
    //customplot3->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);//可读性优于设置
    customplot3->xAxis2->setRange(0, 161);
    customplot3->xAxis2->setTickLabelRotation(60);
    customplot3->xAxis2->setVisible(true);
#endif

    customplot3->yAxis->setRange(0, 1000);

    subBars = new QCPBars(customplot3->xAxis, customplot3->yAxis);
    //subBars->setPen(QPen(Qt::black));
    subBars->setAntialiased(false);
    subBars->setAntialiasedFill(false);
    //subBars->setWidth(0.6);
    subBars->setBrush(QColor("#705BE8"));
    subBars->keyAxis()->setSubTicks(false);

    QFont font1;
    font1.setPixelSize(9);//文字像素大小    
    customplot3->xAxis->setTickLabelFont(font1);
    customplot3->xAxis2->setTickLabelFont(font1);

    //QVector<double> y1;
    //for (int i = 0; i < 176; i++)
    //{
    //    y1 << i + 1;
    //}
    //subBars->setData(ticks, y1, true);
    //customplot3->replot();

    //double energy_accumulation[16] = { 1000,2200,3000,4000,5000,6000,7000,8000,9000,1000,2000,3000,4000,5000,6000,7000 };


}

void DigitalArrays::do_update_customPlot(double** realtime_values, double* FFT_keys, double* FFT_values, double* energy_accumulation, uint32_t length, bool is_changed_xAxis, uint32_t mode)
{
    // 50k 5s 5*50000/2048 = 122 122/5=24 1s

    if (mode == 1)
    {
#ifdef NODE_11_50K_SAMPLE
        static int update_altitude_press_cnt = 0;
        static int sympol_press;
        static double press_sensor_data = 0;
        update_altitude_press_cnt++;

        if (update_altitude_press_cnt == 24)
        {
            //double* address1 = realtime_values[trans2Idx(4, 18)];
            //double* address2 = realtime_values[trans2Idx(4, 17)];
            if (TransForm_press_sensor(realtime_values[trans2Idx(4, 18)][0], realtime_values[trans2Idx(4, 17)][0], &press[0], &sympol_press))
            {
                QString str1 = QString("press:%1 %2").arg(press[0]).arg(press_symbol[sympol_press]);
                pressLineEdit[0]->setText(str1);
            }
            if (TransForm_press_sensor(realtime_values[trans2Idx(8, 18)][0], realtime_values[trans2Idx(8, 17)][0], &press[1], &sympol_press))
            {
                QString str2 = QString("press:%1 %2").arg(press[1]).arg(press_symbol[sympol_press]);
                pressLineEdit[1]->setText(str2);
            }
            if (TransForm_press_sensor(realtime_values[trans2Idx(9, 18)][0], realtime_values[trans2Idx(9, 17)][0], &press[2], &sympol_press))
            {
                QString str3 = QString("press:%1 %2").arg(press[2]).arg(press_symbol[sympol_press]);
                pressLineEdit[2]->setText(str3);
            }
            if (TransForm_press_sensor(realtime_values[trans2Idx(10, 18)][0], realtime_values[trans2Idx(10, 17)][0], &press[3], &sympol_press))
            {
                QString str4 = QString("press:%1 %2").arg(press[3]).arg(press_symbol[sympol_press]);
                pressLineEdit[3]->setText(str4);
            }

            if (TransForm_altitude_sensor(realtime_values[trans2Idx(4, 18)][0], realtime_values[trans2Idx(4, 17)][0], altitude_data[0]))
            {
                QString str1 = QString("roll:%1  pitch:%2  yaw:%3  ").arg(QString::number(altitude_data[0][0], 'f', 2)).arg(QString::number(altitude_data[0][1], 'f', 2)).arg(QString::number(altitude_data[0][2], 'f', 2));
                altitudeLineEdit[0]->setText(str1);
            }
            if (TransForm_altitude_sensor(realtime_values[trans2Idx(8, 18)][0], realtime_values[trans2Idx(8, 17)][0], altitude_data[1]))
            {
                QString str2 = QString("roll:%1  pitch:%2  yaw:%3  ").arg(QString::number(altitude_data[1][0], 'f', 2)).arg(QString::number(altitude_data[1][1], 'f', 2)).arg(QString::number(altitude_data[1][2], 'f', 2));
                altitudeLineEdit[1]->setText(str2);
            }
            if (TransForm_altitude_sensor(realtime_values[trans2Idx(9, 18)][0], realtime_values[trans2Idx(9, 17)][0], altitude_data[2]))
            {
                QString str3 = QString("roll:%1  pitch:%2  yaw:%3  ").arg(QString::number(altitude_data[2][0], 'f', 2)).arg(QString::number(altitude_data[2][1], 'f', 2)).arg(QString::number(altitude_data[2][2], 'f', 2));
                altitudeLineEdit[2]->setText(str3);
            }
            if (TransForm_altitude_sensor(realtime_values[trans2Idx(10, 18)][0], realtime_values[trans2Idx(10, 17)][0], altitude_data[3]))
            {
                QString str4 = QString("roll:%1  pitch:%2  yaw:%3  ").arg(QString::number(altitude_data[3][0], 'f', 2)).arg(QString::number(altitude_data[3][1], 'f', 2)).arg(QString::number(altitude_data[3][2], 'f', 2));
                altitudeLineEdit[3]->setText(str4);
            }
            update_altitude_press_cnt = 0;
        }
#endif

        if (is_changed_xAxis)
        {
            customplot1->xAxis->setRange(0, length * sample_num2time, Qt::AlignLeft);
        }
        for (int i = 0; i < 8; i++)
        {
            Customplot1_graph[i]->addData(realtime_values[trans2Idx(board_add_channel[i][0], board_add_channel[i][1])], length, sample_num2time, true);
        }
        customplot1->replot(QCustomPlot::rpQueuedReplot);

        customplot2->graph(0)->addData(FFT_keys, FFT_values, fft_len_adaptive_sample >> 1, true);
        customplot2->replot(QCustomPlot::rpQueuedReplot);
    }

    else if (mode == 2)
    {
#ifdef NODE_18
        int boardsNum_temp = 2;
#endif
#ifdef NODE_11_50K_SAMPLE
        int boardsNum_temp = 10;
#elif NODE_4_ADAPTIVE_SAMPLE
        int boardsNum_temp = 4;
#endif

        y1.clear();
        for (int i = 0; i < boardsNum_temp * 16; i++)
        {
            y1 << energy_accumulation[i];
            energy_accumulation[i] = 0;
        }
        subBars->setData(ticks, y1, true);
        customplot3->replot(QCustomPlot::rpQueuedReplot);
    }

    else
    {

    }

    //customplot3->replot();
    //for (int i = 0; i < boardsNum_temp * 16; i++)
    //{
    //    //energy_accumulation[i] = 0;
    //}
}

//void DigitalArrays::do_update_customPlot_energy(double* energy_accumulation)
//{

//}


void DigitalArrays::getDiskInfo()
{
    int i = 0;
    QList<QStorageInfo> list = QStorageInfo::mountedVolumes();
    char chr = string_save_path.at(0);
    for (i = 0; i < list.size(); ++i)
    {
        if (list[i].rootPath().contains(chr))
        {
            break;
        }
    }

    QStorageInfo diskInfo = list.at(i);

    freeSize = diskInfo.bytesFree();
    totalSize = diskInfo.bytesTotal();
    QString str = QString::number(freeSize / (1024 * 1024 * 1024), 'f', 3);
    str.append(" GB");
    ui.lEdit_AvailableCap->setText(str);
    //ui.lEdit_attitude1->setText(str);
    ui.lEdit_AvailableCap->setText(str);
    emit newPoint(freeSize, totalSize);
}

void DigitalArrays::do_valueChanged(qint64 data1, qint64 data2)
{//自定义槽函数
    ui.pBar_storage->setValue(100 * data1 / data2);//设置进度条的 value
}


void DigitalArrays::pButton_get_Channels_Clicked()
{
    QString curPath = QDir::currentPath();
    QString dlgTitle = "Select the files"; //对话框标题
    QString filter = "txt file(*.txt);;csv file(*.csv);;all file(*.*)";
    QString fileName = QFileDialog::getOpenFileName(this, dlgTitle, curPath, filter);
    if (!fileName.isEmpty())
        ui.lineEdit_Channels->setText(fileName);
}


void DigitalArrays::pButton_get_SavePath_Clicked()
{
    QString curPath = QCoreApplication::applicationDirPath();
    QString dlgTitle = "choose a dialog";
    QString selectedDir = QFileDialog::getExistingDirectory(this, dlgTitle, curPath);
    if (!selectedDir.isEmpty())
        ui.lineEdit_SavePath->setText(selectedDir);

    selectedDir = QDir::toNativeSeparators(selectedDir);

    //qDebug() << "path:" << selectedDir;
    string_save_path = selectedDir.toStdString();
    Save_path = new char[strlen(string_save_path.c_str()) + 1];
    strcpy(Save_path, string_save_path.c_str());

    _labInfo->setText("System is initializing, please wait a moment");
    QTime _Timer = QTime::currentTime().addMSecs(20000);
    while (QTime::currentTime() < _Timer)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    _labInfo->setText("System initialization completed");
    // 使能某些控件
    ui.pButton_connect->setEnabled(true);
    ui.pButton_getStroage->setEnabled(true);

    // 使某些控件失效

}

void DigitalArrays::comboBox_saveSize()
{
    int num = ui.comboBox->currentIndex();
    switch (num)
    {
    case 0:saveFileNum = 10;
    case 1:saveFileNum = 20;
    case 2:saveFileNum = 30;
    case 3:saveFileNum = 120;
    case 4:saveFileNum = 600;
    default:break;
    }
}

void DigitalArrays::pButtton_test()
{

    getDiskInfo();

    //const double press_point[4] = { 1, 0.1, 0.01, 0.001 };
    //double altitude_sensor_data[3] = { 0 };
    //double press_sensor_data = 0;
    //int* symbol = new int;
    //TransForm_altitude_sensor(0x40004000, 0x100b000, altitude_sensor_data);
    //qDebug() << "altitude_sensor_datax" << altitude_sensor_data[0];
    //qDebug() << "altitude_sensor_datay" << altitude_sensor_data[1];
    //qDebug() << "altitude_sensor_dataz" << altitude_sensor_data[2];
    //TransForm_press_sensor(0x02020001, 0x300b000, &press_sensor_data, symbol);
    //qDebug() << "press_sensor_data" << press_sensor_data << press_symbol[*symbol];

    //qDebug() << "sizeof(qreal)" << sizeof(qreal);
    //qDebug() << "sizeof(float)" << sizeof(float);
    //qDebug() << "sizeof(uint32_t)" << sizeof(uint32_t);
    //qDebug() << "sizeof(long)" << sizeof(long);
    //qDebug() << "sizeof(short)" << sizeof(short);             //16
    //qDebug() << "sizeof(qint64)" << sizeof(qint64);           //64
}




void DigitalArrays::pButton_connectDevice()
{

    ui.pButton_connect->setEnabled(false);
    DUThread = new DoUpdateThread(xAxis_num);

    // 更新绘图
    connect(DUThread, &DoUpdateThread::update_customPlot, this, &DigitalArrays::do_update_customPlot);

    connect(this, &DigitalArrays::fft_board_changed, DUThread, &DoUpdateThread::do_fft_board_changed);
    connect(this, &DigitalArrays::fft_channel_changed, DUThread, &DoUpdateThread::do_fft_channel_changed);
    connect(this, &DigitalArrays::energy_board_changed, DUThread, &DoUpdateThread::do_energy_board_changed);
    connect(this, &DigitalArrays::xAxis_changed, DUThread, &DoUpdateThread::do_xAxis_changed);
    connect(ui.tabWidget, &QTabWidget::currentChanged, this, &DigitalArrays::tabWidget_IdxChanged);
    connect(this, &DigitalArrays::tabwidget_init_idx, this, &DigitalArrays::tabWidget_IdxChanged);
    emit(tabwidget_init_idx(1));

    _labInfo->setText("connect operation is running");

    QVector<QCPGraphData> plotData(xAxis_num);
    for (size_t j = 0; j < xAxis_num; j++)
    {
        plotData[j].key = j * sample_num2time;
        plotData[j].value = 0;
}

    QSharedPointer<QCPGraphDataContainer> dataContainer[8];
    for (int i = 0; i < 8; i++)
    {
        dataContainer[i] = customplot1->graph(i)->data();
        dataContainer[i]->set(plotData, true);
    }

    mData_buffer_realtime = customplot1->graph(0)->data()->coreData();

    for (int i = 0; i < 8; i++)
    {
        Customplot1_graph[i]->setVisible(false);
    }
    Customplot1_graph[0]->setVisible(true);
    customplot1->replot(QCustomPlot::rpQueuedReplot);

#ifdef NODE_11_50K_SAMPLE
    int bNum = 11;
#elif NODE_4_ADAPTIVE_SAMPLE
    int bNum = 4;
#endif
    GDThread = new GetDataThread(512, bNum, xAxis_num * 10, this);
    connect(GDThread, &GetDataThread::trans_ringbuffer, DUThread, &DoUpdateThread::do_trans_ringbuffer);
    GDThread->emit_signal();

    char s[] = "tong dao ying she biao";
    if (!init(bNum, s, Save_path, saveFileNum, sampleRate))
    {
        _labInfo->setText("connect operation is failed, please reboot!");
    }
    else
    {
        QTime _Timer = QTime::currentTime().addMSecs(200);
        while (QTime::currentTime() < _Timer)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }

        if (!GDThread->isRunning())
        {
            GDThread->start();
        }
        GDThread->beginThread();

        if (!DUThread->isRunning())
        {
            DUThread->start();
        }
        DUThread->beginThread();

        QTime _Timer1 = QTime::currentTime().addMSecs(2000);
        while (QTime::currentTime() < _Timer1)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
        _labInfo->setText("Connect Operation Command Send Successful");
        // 使能某些控件
#ifdef NODE_11_50K_SAMPLE
        ui.pButton_selfcheck->setEnabled(true);
#endif



    ui.graph1->setEnabled(true);
    ui.graph2->setEnabled(true);
    ui.graph3->setEnabled(true);
    ui.graph4->setEnabled(true);
    ui.graph5->setEnabled(true);
    ui.graph6->setEnabled(true);
    ui.graph7->setEnabled(true);
    ui.graph8->setEnabled(true);

        ui.pButton_systembegin->setEnabled(true);
        //ui.cBox_xAxis->setEnabled(true);
        //ui.spinBox_fft_board->setEnabled(true);
        //ui.spinBox_fft_channel->setEnabled(true);
        //ui.spinBox_energy_board->setEnabled(true);
        //ui.pButton_begin->setEnabled(true);
        //ui.pButton_save->setEnabled(true);

        // 使某些控件失效
        ui.pButton_connect->setEnabled(false);
        ui.pButton_get_SavePath->setEnabled(false);
        ui.comboBox->setEnabled(false);
        ui.spinBox_num_of_boards->setEnabled(false);
        ui.pButton_get_Channels->setEnabled(false);
        ui.comboBox_sampleRate->setEnabled(false);
    }
}

// graph1
void DigitalArrays::checkBox_graph1(bool isVisable)
{
    if (!isVisable)
    {
        Customplot1_graph[0]->setVisible(false);
    }
    else
    {
        Customplot1_graph[0]->setVisible(true);
    }
    customplot1->replot();
}
// graph2
void DigitalArrays::checkBox_graph2(bool isVisable)
{
    if (!isVisable)
    {
        Customplot1_graph[1]->setVisible(false);
    }
    else
    {
        Customplot1_graph[1]->setVisible(true);
    }
    customplot1->replot();
}
// graph3
void DigitalArrays::checkBox_graph3(bool isVisable)
{
    if (!isVisable)
    {
        Customplot1_graph[2]->setVisible(false);
    }
    else
    {
        Customplot1_graph[2]->setVisible(true);
    }
    customplot1->replot();
}
// graph4
void DigitalArrays::checkBox_graph4(bool isVisable)
{
    if (!isVisable)
    {
        Customplot1_graph[3]->setVisible(false);
    }
    else
    {
        Customplot1_graph[3]->setVisible(true);
    }
    customplot1->replot();
}
// graph5
void DigitalArrays::checkBox_graph5(bool isVisable)
{
    if (!isVisable)
    {
        Customplot1_graph[4]->setVisible(false);
    }
    else
    {
        Customplot1_graph[4]->setVisible(true);
    }
    customplot1->replot();
}
// graph6
void DigitalArrays::checkBox_graph6(bool isVisable)
{
    if (!isVisable)
    {
        Customplot1_graph[5]->setVisible(false);
    }
    else
    {
        Customplot1_graph[5]->setVisible(true);
    }
    customplot1->replot();
}
// graph7
void DigitalArrays::checkBox_graph7(bool isVisable)
{
    if (!isVisable)
    {
        Customplot1_graph[6]->setVisible(false);
    }
    else
    {
        Customplot1_graph[6]->setVisible(true);
    }
    customplot1->replot();
}
// graph8
void DigitalArrays::checkBox_graph8(bool isVisable)
{
    if (!isVisable)
    {
        Customplot1_graph[7]->setVisible(false);
    }
    else
    {
        Customplot1_graph[7]->setVisible(true);
    }
    customplot1->replot();
}

void DigitalArrays::sBox_num_of_boards_valueChanged(int value)
{
    Boards_nums = value;
}

void DigitalArrays::sBox_fft_board_valueChanged(int value)
{
    emit(fft_board_changed(value));
}

void DigitalArrays::sBox_fft_channel_valueChanged(int value)
{
    emit(fft_channel_changed(value));
}

void DigitalArrays::sBox_energy_board_valueChanged(int value)
{
    emit(energy_board_changed(value));
}

void DigitalArrays::do_spinbox_value_changed(int value)
{
    QSpinBox* sbox = qobject_cast<QSpinBox*>(sender());
    for (int i = 0; i < 8; i++)
    {
        //QMutexLocker lck(&mtx_board_channal_select);
        //{
        if (board_sbox[i] == sbox)
        {
            board_add_channel[i][0] = value;
            //qDebug() << "board_sbox:" << i + 1 << " value: " << value;
        }
        if (channel_sbox[i] == sbox)
        {
            board_add_channel[i][1] = value;
            //qDebug() << "channel_sbox:" << i + 1 << " value: " << value;
        }
        //}
    }
}

void DigitalArrays::comboBox_xAxis(int idx)
{
    int num = ui.cBox_xAxis->currentIndex();
    switch (num) {
    case 0:
        num = xAxis_num; break;
    case 1:
        num = xAxis_num * 10; break;
    default:
        num = xAxis_num; break;
    }
    //customplot1->xAxis->setRange(0, num, Qt::AlignLeft);
    //customplot1->replot();
    emit(xAxis_changed(num));
    //qDebug() << num;
}

void DigitalArrays::sBox_yAxis_valueChanged(double value)
{
    customplot1->yAxis->setRange(-value, value);
    customplot1->replot();
    //qDebug() << value;

}

void DigitalArrays::comboBox_sampleRateChanged(int value)
{
    switch (value)
    {
    case 0:sampleRate = 50000; xAxis_num = 5000; sample_num2time = SAMPLE_CNT_TO_TIME_50K; fft_len_adaptive_sample = QCUSTOMPLOT_FLUSH_50K_COUNT; break;   // 50k
    case 1:sampleRate = 25000; xAxis_num = 2500; sample_num2time = SAMPLE_CNT_TO_TIME_25K; fft_len_adaptive_sample = QCUSTOMPLOT_FLUSH_25K_COUNT; break;   // 25k
    case 2:sampleRate = 100000; xAxis_num = 10000; sample_num2time = SAMPLE_CNT_TO_TIME_100K; fft_len_adaptive_sample = QCUSTOMPLOT_FLUSH_100K_COUNT; break;   // 100k 参看tcp与ps协议
    default:break;
    }


    customplot1->xAxis->setRange(0, xAxis_num * sample_num2time, Qt::AlignLeft);
    

    //qDebug() << "sampleRate:" << sampleRate;
}

void DigitalArrays::sBox_yAxis_fft_valueChanged(double value)
{
    customplot2->yAxis->setRange(0, value);
    customplot2->replot();
}

void DigitalArrays::sBox_yAxis_energy_valueChanged(double value)
{
    customplot3->yAxis->setRange(0, value);
    customplot3->replot();
}

void DigitalArrays::pButton_selfcheck()
{
    ui.pButton_selfcheck->setEnabled(false);
    ui.pButton_systembegin->setEnabled(false);
    _labInfo->setText("self-check is ongoing, please do not operate");

    QTime _Timer = QTime::currentTime().addMSecs(1000);
    while (QTime::currentTime() < _Timer)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }



    if (!send_msg(CMD_SELFCHECK))
    {
        _labInfo->setText("self-check cmd send error, please reboot the system");
    }
    else
    {

        while (!(get_selfcheck_state_for_qt() >> 7))
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
            uint32_t num = get_selfcheck_state_for_qt();
            //QCoreApplication::processEvents();
            //QThread::msleep(500);
        }

        uint32_t selfcheck_state_for_qt = trans_selfcheck_state_for_qt(get_selfcheck_state_for_qt());


        //ui.pButton_selfcheck->setEnabled(false);

        system_state_group_indicatorLight[23]->setState(selfcheck_state_for_qt & 0x1);
        system_state_group_indicatorLight[22]->setState(selfcheck_state_for_qt >> 1 & 0x1);
        for (int i = 0; i < 11; i++)
        {
            system_state_group_indicatorLight[21 - i * 2]->setState((selfcheck_state_for_qt >> (i * 2 + 2)) & 0x1);
            system_state_group_indicatorLight[21 - i * 2 - 1]->setState((selfcheck_state_for_qt >> (i * 2 + 3)) & 0x1);
        }

        QTime _Timer = QTime::currentTime().addMSecs(1000);
        while (QTime::currentTime() < _Timer)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }

        _labInfo->setText("self-check over");
        ui.pButton_systembegin->setEnabled(true);
    }
}

void DigitalArrays::pButton_systemEnable()
{
    ui.pButton_selfcheck->setEnabled(false);
    ui.pButton_systembegin->setEnabled(false);
    if (!send_msg(CMD_BEGIN))
    {
        _labInfo->setText("systemEnable cmd send error, please reboot the system");
    }
    else
    {
        QTime _Timer = QTime::currentTime().addMSecs(5000);
        while (QTime::currentTime() < _Timer)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }

        _labInfo->setText("system is running");
        ui.pButton_systembegin->setEnabled(false);

        ui.cBox_xAxis->setEnabled(true);
        ui.spinBox_fft_board->setEnabled(true);
        ui.spinBox_fft_channel->setEnabled(true);
        //ui.spinBox_energy_board->setEnabled(true);
        ui.pButton_begin->setEnabled(true);
        ui.pButton_save->setEnabled(true);

        // 启动实时数据
        indicatorLight_realtime->setState(true);
        DUThread->realTimeDisplay(true);
    }

}

// 实时数据按钮
void DigitalArrays::pButton_begin()
{
    static bool display_flag = false;
    
    indicatorLight_realtime->setState(display_flag);
    DUThread->realTimeDisplay(display_flag);

    display_flag = !display_flag;
}

// 保存数据按钮
void DigitalArrays::pButton_save()
{
    static bool save_flag = true;

    indicatorLight_savefiles->setState(save_flag);
    begin_save(save_flag);

    save_flag = !save_flag;
}

void DigitalArrays::tabWidget_IdxChanged(int idx)
{
    tabWidget_idx = idx;
    DUThread->setTabWidgetIdx(idx);
}
