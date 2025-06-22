#include <QApplication>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qDebug() << "Greeter is exit.";
    return app.exec();
}
