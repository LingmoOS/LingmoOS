#ifndef ROTATECHANGEINFO_H
#define ROTATECHANGEINFO_H

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
#include <QScreen>
#include <QPair>

static const QString LINGMO_ROTATION_PATH = "/";
static const QString LINGMO_ROTATION_SERVICE = "com.lingmo.statusmanager.interface";
static const QString LINGMO_ROTATION_INTERFACE = "com.lingmo.statusmanager.interface";

class RotateChangeInfo : public QObject
{
    Q_OBJECT
public:
    explicit RotateChangeInfo(QObject *parent = nullptr);
    QPair<int,int> getCurrentScale();
    bool getCurrentMode();
private:
    int m_screenWidth;
    int m_screenHeight;
    bool m_isPCMode;

signals:
    void sigRotationChanged(bool mode, int scale_W, int scale_H); // 信号发出模式，宽大小，高大小
    
public slots:
    void slotRotateChanged();
    void slotModeChanged(bool isPadMode);

};

#endif // ROTATECHANGEINFO_H
