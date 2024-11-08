#include <QApplication>
#include <QTranslator>
#include "dialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // sdk翻译加载
    QTranslator trans;
    if(trans.load(":/translations/gui_"+QLocale::system().name()+".qm"))
    {
        a.installTranslator(&trans);
    }

    Dialog *testDailog = new Dialog();

    return a.exec();
}
