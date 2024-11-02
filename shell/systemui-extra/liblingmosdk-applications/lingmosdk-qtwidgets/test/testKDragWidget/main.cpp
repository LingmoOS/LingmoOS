#include "widget.h"
#include "kwidgetutils.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    kdk::KWidgetUtils::highDpiScaling();
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
