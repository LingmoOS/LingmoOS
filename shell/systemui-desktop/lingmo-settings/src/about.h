#ifndef ABOUT_H
#define ABOUT_H

#include <QObject>
#include <QString>
#include <QSysInfo>
#include <qqml.h>

class About : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLingmoOS READ isLingmoOS CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString hostName READ hostName CONSTANT)
    Q_PROPERTY(QString systemrelease READ systemrelease CONSTANT)
    Q_PROPERTY(QString debianversion READ debianversion CONSTANT)
    Q_PROPERTY(QString desktopversion READ desktopversion CONSTANT)
    Q_PROPERTY(QString buildtime READ buildtime CONSTANT)
    Q_PROPERTY(QString buildversion READ buildversion CONSTANT)
    Q_PROPERTY(QString uiversion READ uiversion CONSTANT)
    Q_PROPERTY(QString updateversion READ updateversion CONSTANT)
    Q_PROPERTY(QString osName READ osName CONSTANT)
    Q_PROPERTY(QString architecture READ architecture CONSTANT)
    Q_PROPERTY(QString kernelVersion READ kernelVersion CONSTANT)
    Q_PROPERTY(QString hostname READ hostname CONSTANT)
    Q_PROPERTY(QString userName READ userName CONSTANT)
    Q_PROPERTY(QString memorySize READ memorySize CONSTANT)
    Q_PROPERTY(QString prettyProductName READ prettyProductName CONSTANT)
    Q_PROPERTY(QString internalStorage READ internalStorage CONSTANT)
    Q_PROPERTY(QString cpuInfo READ cpuInfo CONSTANT)

public:
    explicit About(QObject *parent = nullptr);

    bool isLingmoOS();
    Q_INVOKABLE bool checkForLingmoOS();

    QString version();
    QString hostName();
    QString systemrelease();
    QString debianversion();
    QString desktopversion();
    QString buildtime();
    QString buildversion();
    QString uiversion();
    QString updateversion();

    QString osName();
    QString architecture();
    QString kernelType();
    QString kernelVersion();
    QString hostname();
    QString userName();
    QString settingsVersion();
    QString memorySize();
    QString prettyProductName();
    QString internalStorage();
    QString cpuInfo();

    Q_INVOKABLE void openUpdator();

private:
    qlonglong calculateTotalRam() const;
};

#endif // ABOUT_H
