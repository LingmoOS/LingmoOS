// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

#include <QObject>
#include <QDBusContext>

class DeviceInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.DeviceInfo")

public:
    explicit DeviceInterface(const char *name, QObject *parent = nullptr);

signals:
    void sigUpdate();

public slots:
    /**
     * @brief getInfo : Obtain hardware information through the DBus
     * @param key
     * @return : Hardware info
     */
    Q_SCRIPTABLE QString getInfo(const QString &key);

    /**
     * @brief refreshInfo
     * @return
     */
    Q_SCRIPTABLE void refreshInfo();

    /**
     * @brief setMonitorDeviceFlag
     * @return
     */
    Q_SCRIPTABLE void setMonitorDeviceFlag(bool flag);

private:
    bool getUserAuthorPasswd();
};

#endif   // DEVICEINTERFACE_H
