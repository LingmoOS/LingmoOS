// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISCOVERYCONTROLLER_H
#define DISCOVERYCONTROLLER_H

#include "global_defines.h"
#include "discover/deviceinfo.h"
#include <QObject>
#include "qzeroconf.h"

typedef QMap<QString, QString> StringMap;
Q_DECLARE_METATYPE(StringMap)

namespace cooperation_core {

class DiscoverControllerPrivate;
class DiscoverController : public QObject
{
    Q_OBJECT
public:
    static DiscoverController *instance();

    void init();

    void publish();
    void unpublish();
    void updatePublish();
    void refresh();
    void startDiscover();

    QList<DeviceInfoPointer> getOnlineDeviceList() const;
    DeviceInfoPointer findDeviceByIP(const QString &ip);
    static DeviceInfoPointer selfInfo();

    void updateDeviceState(const DeviceInfoPointer info);

    static bool isZeroConfDaemonActive();
    static bool openZeroConfDaemonDailog();

Q_SIGNALS:
    void deviceOnline(const QList<DeviceInfoPointer> &infoList);
    void deviceOffline(const QString &ip);
    void startDiscoveryDevice();
    void discoveryFinished(bool hasFound);

    void registCompatAppInfo(bool reg, const QString &infoJson);


public Q_SLOTS:
    void updateHistoryDevices(const QMap<QString, QString> &connectMap);

private Q_SLOTS:
    void addService(QZeroConfService zcs);
    void removeService(QZeroConfService zcs);
    void updateService(QZeroConfService zcs);

    void onDConfigValueChanged(const QString &config, const QString &key);
    void onAppAttributeChanged(const QString &group, const QString &key, const QVariant &value);

    void addSearchDeivce(const QString &info);
    void compatAddDeivces(StringMap infoMap);
    void compatRemoveDeivce(const QString &ip);

private:
    explicit DiscoverController(QObject *parent = nullptr);
    ~DiscoverController();

    void initZeroConf();
    void initConnect();
    bool isVaildDevice(const DeviceInfoPointer info);
    DeviceInfoPointer parseDeviceJson(const QString &info);
    DeviceInfoPointer parseDeviceService(QZeroConfService zcs);
    void deviceLosted(const QString &ip);

private:
    QSharedPointer<DiscoverControllerPrivate> d { nullptr };

    QString _connectedDevice;
    QStringList _historyDevices;
};

}   // namespace cooperation_core

#endif   // DISCOVERYCONTROLLER_H
