#include "lingmo_media_control_led.h"
#include <QFileInfo>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QProcess>
#include <QDir>

LingmoUIMediaControlLed::LingmoUIMediaControlLed()
{
    QDBusConnection::systemBus().connect(QString(),"/","org.lingmo.media","systemOutputVolumeIsMute",this,SLOT(outputMuteChanged(QString)));
    QDBusConnection::systemBus().connect(QString(),"/","org.lingmo.media","systemInputVolumeIsMute",this,SLOT(inputMuteChanged(QString)));
}

void LingmoUIMediaControlLed::outputMuteChanged(QString str)
{
    QFile file("/sys/class/leds/platform::mute/brightness");

    if(!file.exists()){
        qDebug() << " THE DIRECTORY OR FILE IS NOT EXIST ";
        return;
    }

    qDebug() << "sink muted" << str;

    if (strstr(str.toLatin1().data(),"mute")) {
        qDebug() << "输出音量静音";
        system("echo 1 > /sys/class/leds/platform::mute/brightness");
    }
    else if (strstr(str.toLatin1().data(),"no")) {
        qDebug() << "输出音量取消静音";
        system("echo 0 > /sys/class/leds/platform::mute/brightness");
    }
}


void LingmoUIMediaControlLed::inputMuteChanged(QString str)
{
    QFile file("/sys/class/leds/platform::micmute/brightness");

    if(!file.exists()){
        qDebug() << " THE DIRECTORY OR FILE IS NOT EXIST ";
        return;
    }

    qDebug() << "source muted" << str;

    if (strstr(str.toLatin1().data(),"mute")) {
        qDebug() << "输入音量静音";
        system("echo 1 > /sys/class/leds/platform::micmute/brightness");

    }
    else if (strstr(str.toLatin1().data(),"no")) {
        qDebug() << "输入音量取消静音";
        system("echo 0 > /sys/class/leds/platform::micmute/brightness");
    }
}


LingmoUIMediaControlLed::~LingmoUIMediaControlLed()
{

}
