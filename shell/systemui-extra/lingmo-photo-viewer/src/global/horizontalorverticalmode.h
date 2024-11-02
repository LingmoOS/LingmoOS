#ifndef HORIZONTALORVERTICALMODE_H
#define HORIZONTALORVERTICALMODE_H

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include <sys/types.h>

enum deviceMode {
    PADHorizontalMode = 1, //平板横屏
    PADVerticalMode = 2,   //平板竖屏
    PCMode = 3             // pc模式
};

class HorizontalOrVerticalMode : public QObject
{
    Q_OBJECT
public:
    HorizontalOrVerticalMode();
    deviceMode defaultModeCapture();
Q_SIGNALS:
    void RotationSig(deviceMode); // true:horizontal;false:vertical
private Q_SLOTS:
    void rotationChanged(QString res);
    void modeChanged(bool res);
};


#endif // HORIZONTALORVERTICALMODE_H
