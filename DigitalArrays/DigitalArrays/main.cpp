#include "DigitalArrays.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    //SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
    //signal(SIGABRT, CrashHandler);
    //signal(SIGFPE, CrashHandler);
    //signal(SIGILL, CrashHandler);
    //signal(SIGINT, CrashHandler);
    //signal(SIGSEGV, CrashHandler);
    //signal(SIGTERM, CrashHandler);

    QApplication a(argc, argv);
    DigitalArrays w;
    w.setWindowState(Qt::WindowMaximized);                      // »´∆¡œ‘ æ
    w.show();

    return a.exec();
}
