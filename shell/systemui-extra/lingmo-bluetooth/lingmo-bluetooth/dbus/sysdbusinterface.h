#ifndef SYSDBUSINTERFACE_H
#define SYSDBUSINTERFACE_H

#include <QStandardPaths>
#include <QObject>
#include <QtDBus/QDBusContext>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>

class sysdbusinterface : public QObject, protected QDBusContext
{
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.bluetooth")
    Q_OBJECT
public:
    sysdbusinterface();
    ~sysdbusinterface();

public slots:
    QString getProperFilePath(QString user, QString filename);

};

#endif // SYSDBUSINTERFACE_H
