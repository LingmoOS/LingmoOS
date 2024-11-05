#include "kt-operation-test.h"

#include <QApplication>
#include <QtTest/QTest>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KTOperationTest kt;
    QTest::qExec(&kt, argc, argv);
    return 0;
}
