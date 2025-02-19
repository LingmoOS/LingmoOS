#include <QApplication>
#include <QScreen>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    MainWindow w;
    // 设置无边框全屏
    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    w.showFullScreen();
    
    return a.exec();
} 