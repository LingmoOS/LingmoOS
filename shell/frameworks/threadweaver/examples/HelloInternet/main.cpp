#include <QApplication>

#include "MainWidget.h"
#include "ViewController.h"

//@@snippet_begin(hellointernet-main)
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w;
    ViewController v(&w);
    w.show();
    return a.exec();
}
//@@snippet_end
