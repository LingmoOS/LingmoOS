#ifndef KCOLORUTILSTEST_H
#define KCOLORUTILSTEST_H

#include <QObject>

class tst_KColorUtils : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testOverlay();
    void testMix();
    void testHCY();
    void testContrast();
    void testShading();
};

#endif // KCOLORUTILSTEST_H
