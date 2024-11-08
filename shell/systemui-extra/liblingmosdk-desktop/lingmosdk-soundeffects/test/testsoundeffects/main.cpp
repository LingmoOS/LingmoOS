#include "widget.h"

#include <QApplication>
#include "kwidgetutils.h"

int main(int argc, char *argv[])
{
    kdk::KWidgetUtils::highDpiScaling();
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
