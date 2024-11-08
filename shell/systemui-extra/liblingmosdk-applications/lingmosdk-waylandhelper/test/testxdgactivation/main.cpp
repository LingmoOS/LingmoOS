#include "widget.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget window1(nullptr, Qt::Window);
    window1.setWindowTitle("Window 1");
    window1.setLayout(new QVBoxLayout);
    QPushButton p("Raise the Window 2");
    window1.layout()->addWidget(&p);
    window1.show();

    QWidget window2(nullptr, Qt::Window);
    window2.setWindowTitle("Window 2");
    window2.show();
    QObject::connect(&p, &QPushButton::clicked, &a, [&] {
    kdk::WindowManager::activateWindow(window1.windowHandle(),window2.windowHandle());});
    return a.exec();
}
