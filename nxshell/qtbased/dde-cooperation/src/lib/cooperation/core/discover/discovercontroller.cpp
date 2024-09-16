// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "discovercontroller.h"
#include "discovercontroller_p.h"
#include "utils/cooperationutil.h"
#include "common/log.h"

#include <QProcess>
#include <QMessageBox>
#include <QTimer>
#include <QJsonDocument>
#include <QDesktopServices>

#include <common/constant.h>
#include <configs/dconfig/dconfigmanager.h>
#include <configs/settings/configmanager.h>

using namespace cooperation_core;

DiscoverControllerPrivate::DiscoverControllerPrivate(DiscoverController *qq)
    : q(qq)
{
}

DiscoverController::DiscoverController(QObject *parent)
    : QObject(parent),
      d(new DiscoverControllerPrivate(this))
{
    qRegisterMetaType<StringMap>("StringMap");
}

DiscoverController::~DiscoverController()
{
}

void DiscoverController::init()
{
    if (isZeroConfDaemonActive()) {
        initZeroConf();
        return;
    }

    openZeroConfDaemonDailog();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [timer, this] {
        if (isZeroConfDaemonActive()) {
            initZeroConf();
            timer->stop();
        }
    });
    timer->start(5000);
}

void DiscoverController::initZeroConf()
{
    d->zeroConf = new QZeroConf();
    d->zeroconfname = QSysInfo::machineUniqueId();

    // publish();
    initConnect();
    // connect signal should before  browser
    if (!d->zeroConf->browserExists())
        d->zeroConf->startBrowser(UNI_CHANNEL);
}

void DiscoverController::initConnect()
{
    connect(CooperationUtil::instance(), &CooperationUtil::onlineStateChanged, this, [this](const QString &validIP) {
        if (validIP.isEmpty())
            return;
        updatePublish();
        startDiscover();
    });
#ifdef linux
    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &DiscoverController::onDConfigValueChanged);
#endif
    connect(ConfigManager::instance(), &ConfigManager::appAttributeChanged, this, &DiscoverController::onAppAttributeChanged);
    connect(d->zeroConf, &QZeroConf::serviceAdded, this, &DiscoverController::addService);
    connect(d->zeroConf, &QZeroConf::serviceRemoved, this, &DiscoverController::removeService);
    connect(d->zeroConf, &QZeroConf::serviceUpdated, this, &DiscoverController::updateService);
}

bool DiscoverController::isVaildDevice(const DeviceInfoPointer info)
{
    if (!info || info->ipAddress().isEmpty() || !info->ipAddress().startsWith(d->ipfilter))
        return false;
    else
        return true;
}

DeviceInfoPointer DiscoverController::parseDeviceJson(const QString &info)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(info.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        WLOG << "error parse device info:" << info.toStdString();
        return nullptr;
    }

    auto map = doc.toVariant().toMap();
    auto devInfo = DeviceInfo::fromVariantMap(map);
    devInfo->setConnectStatus(DeviceInfo::Connectable);

    return devInfo;
}

DeviceInfoPointer DiscoverController::parseDeviceService(QZeroConfService zcs)
{
    QVariantMap infomap;
    for (const auto &key : zcs->txt().keys())
        infomap.insert(key, QString::fromUtf8(QByteArray::fromBase64(zcs->txt().value(key))));

    auto devInfo = DeviceInfo::fromVariantMap(infomap);
    if (!isVaildDevice(devInfo))
        return nullptr;

    auto old = findDeviceByIP(devInfo->ipAddress());
    if (old) {
        // update its status
        devInfo->setConnectStatus(old->connectStatus());
    }

    return devInfo;
}

QList<DeviceInfoPointer> DiscoverController::getOnlineDeviceList() const
{
    return d->onlineDeviceList;
}

bool DiscoverController::openZeroConfDaemonDailog()
{
#ifdef __linux__
    CooperationDialog dlg;
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.addButton(tr("Confirm"), true, CooperationDialog::ButtonRecommend);
    dlg.addButton(tr("Close"), false, CooperationDialog::ButtonWarning);

    dlg.setTitle(tr("Please click to confirm to enable the LAN discovery service!"));
    dlg.setMessage(tr("Unable to discover and be discovered by other devices when LAN discovery service is not turned on"));

    int code = dlg.exec();
    if (code == 0)
        QProcess::startDetached("systemctl start avahi-daemon.service");
    return true;
#else
    int choice = QMessageBox::warning(nullptr, tr("Please click to confirm to enable the LAN discovery service!"),
                                      tr("Unable to discover and be discovered by other devices when LAN discovery service is not turned on"
                                         "Right click on Windows Start menu ->Computer Management ->Services and Applications ->Services to enable Bonjour service"),
                                      QMessageBox::Ok);
    QDesktopServices::openUrl(QUrl("services.msc"));
    return false;
#endif
}

bool DiscoverController::isZeroConfDaemonActive()
{
#ifdef __linux__
    QProcess process;
    process.start("systemctl", QStringList() << "is-active"
                                             << "avahi-daemon.service");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        if (output.contains("active")) {
            LOG << "Avahi service is running";
            return true;
        } else {
            WLOG << "Avahi service is not running";
            return false;
        }
    } else {
        ELOG << "Error: " << error.toStdString();
        return false;
    }

#else
    QProcess process;
    process.start("sc query \"Bonjour Service\"");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString res = QString::fromLocal8Bit(output);

    if (res.contains("RUNNING"))
        return true;
    else
        return false;
#endif
}

DeviceInfoPointer DiscoverController::findDeviceByIP(const QString &ip)
{
    for (int i = 0; i < d->onlineDeviceList.size(); ++i) {
        auto info = d->onlineDeviceList[i];
        if (info->ipAddress() == ip)
            return info;
    }
    return nullptr;
}

DeviceInfoPointer DiscoverController::selfInfo()
{
    return DeviceInfo::fromVariantMap(CooperationUtil::deviceInfo());
}

void DiscoverController::updateDeviceState(const DeviceInfoPointer info)
{
    auto oldinfo = findDeviceByIP(info->ipAddress());
    if (oldinfo)
        d->onlineDeviceList.removeOne(oldinfo);

    if (DeviceInfo::Connected == info->connectStatus()) {
        //record the connected status IP
        _connectedDevice = info->ipAddress();
    } else {
        _connectedDevice = "";
    }

    d->onlineDeviceList.append(info);
    Q_EMIT deviceOnline({ info });
}

void DiscoverController::onDConfigValueChanged(const QString &config, const QString &key)
{
    Q_UNUSED(key);
    if (config != kDefaultCfgPath)
        return;

    updatePublish();
}

void DiscoverController::onAppAttributeChanged(const QString &group, const QString &key, const QVariant &value)
{
    if (group != AppSettings::GenericGroup)
        return;

    if (key == AppSettings::StoragePathKey)
        CooperationUtil::instance()->setStorageConfig(value.toString());

    updatePublish();
}

void DiscoverController::addService(QZeroConfService zcs)
{
    if (zcs.get()->name() == d->zeroconfname) {
        LOG << "add service, ignore self zcs service";
        return;
    }
    auto devInfo = parseDeviceService(zcs);

    if (!devInfo)
        return;

    d->onlineDeviceList.append(devInfo);
    Q_EMIT deviceOnline({ devInfo });
}

void DiscoverController::updateService(QZeroConfService zcs)
{
    if (zcs.get()->name() == d->zeroconfname) {
        LOG << "update service, ignore self zcs service";
        return;
    }
    auto devInfo = parseDeviceService(zcs);

    if (!devInfo)
        return;
    auto oldinfo = findDeviceByIP(devInfo->ipAddress());
    if (oldinfo)
        d->onlineDeviceList.removeOne(oldinfo);

    d->onlineDeviceList.append(devInfo);
    Q_EMIT deviceOnline({ devInfo });
}

void DiscoverController::removeService(QZeroConfService zcs)
{
    auto devInfo = parseDeviceService(zcs);

    if (!devInfo)
        return;

    auto oldinfo = findDeviceByIP(devInfo->ipAddress());
    if (oldinfo)
        d->onlineDeviceList.removeOne(oldinfo);

    Q_EMIT deviceOffline(devInfo->ipAddress());
}

void DiscoverController::updateHistoryDevices(const QMap<QString, QString> &connectMap)
{
    QList<DeviceInfoPointer> offlineDevList;
    auto iter = connectMap.begin();
    for (; iter != connectMap.end(); ++iter) {
        if (findDeviceByIP(iter.key()))
            continue;

        DeviceInfoPointer info(new DeviceInfo(iter.key(), iter.value()));
        info->setConnectStatus(DeviceInfo::Offline);
        offlineDevList << info;
    }

    if (!offlineDevList.isEmpty())
        Q_EMIT deviceOnline(offlineDevList);
}

DiscoverController *DiscoverController::instance()
{
    static DiscoverController ins;
    return &ins;
}

void DiscoverController::publish()
{
    d->zeroConf->clearServiceTxtRecords();

    QVariantMap deviceInfo = CooperationUtil::deviceInfo();
    //设置为局域网不发现
    if (deviceInfo.value(AppSettings::DiscoveryModeKey) == 1) {
        unpublish();
        return;
    }

    QString selfIP = deviceInfo.value(AppSettings::IPAddress).toString();
    d->ipfilter = selfIP.lastIndexOf(".") != -1 ? selfIP.left(selfIP.lastIndexOf(".")) : "";

    LOG << "publish " << d->zeroconfname.toStdString() << " on: " << selfIP.toStdString();
    for (const auto &key : deviceInfo.keys())
        d->zeroConf->addServiceTxtRecord(key, deviceInfo.value(key).toString().toUtf8().toBase64());

    d->zeroConf->startServicePublish(d->zeroconfname.toUtf8(), UNI_CHANNEL, "local", UNI_RPC_PORT_UDP);

    auto doc = QJsonDocument::fromVariant(deviceInfo);
    Q_EMIT registCompatAppInfo(true, doc.toJson());
}

void DiscoverController::unpublish()
{
    d->zeroConf->stopServicePublish();
    Q_EMIT registCompatAppInfo(false, "");
}

void DiscoverController::updatePublish()
{
    if (!d->zeroConf)
        return;

    unpublish();
    publish();
}

void DiscoverController::refresh()
{
    if (!d->zeroConf)
        return;

    d->onlineDeviceList.clear();
    auto allServices = d->zeroConf->getServices();

    for (const auto &key : allServices.keys()) {
        QZeroConfService zcs = allServices.value(key);
        auto devInfo = parseDeviceService(zcs);

        if (devInfo && devInfo->ipAddress() != CooperationUtil::localIPAddress()) {
            if (_connectedDevice == devInfo->ipAddress()) {
                devInfo->setConnectStatus(DeviceInfo::Connected);
            }
            d->onlineDeviceList.append(devInfo);
        }
    }
    if (d->searchDevice)
        d->onlineDeviceList.append(d->searchDevice);

    Q_EMIT deviceOnline({ d->onlineDeviceList });
    bool hasFound = !d->onlineDeviceList.isEmpty();
    Q_EMIT discoveryFinished(hasFound);
}

void DiscoverController::addSearchDeivce(const QString &info)
{
    auto devInfo = parseDeviceJson(info);
    if (!devInfo) {
        Q_EMIT discoveryFinished(false);
        return;
    }
    d->searchDevice = devInfo;
    if (devInfo->isValid())
        Q_EMIT deviceOnline({ d->searchDevice });
}

void DiscoverController::compatAddDeivces(StringMap infoMap)
{
    QList<DeviceInfoPointer> addedList;
    for (auto it = infoMap.constBegin(); it != infoMap.constEnd(); ++it) {
        QString info = it.key();

        auto devInfo = parseDeviceJson(info);
        if (!devInfo) {
            WLOG << "Can not parse peer: " << info.toStdString();
            continue;
        }

        // 解析 combinedIP
        QString combinedIP = it.value();
        QStringList ipList = combinedIP.split(", ");
        if (ipList.size() != 2) {
            WLOG << "Invalid combined IP format: " << combinedIP.toStdString();
            continue;
        }

        QString ip = ipList[0];
        QString sharedip = ipList[1];

        if(ip == CooperationUtil::localIPAddress()) {
            WLOG << "Ignore local host ip: " << ip.toStdString();
            continue;
        }

        devInfo->setIpAddress(ip);
        if (devInfo->discoveryMode() == DeviceInfo::DiscoveryMode::Everyone) {
            if (sharedip == CooperationUtil::localIPAddress())
                devInfo->setConnectStatus(DeviceInfo::Connected);

            d->onlineDeviceList.append(devInfo);
            addedList.append(devInfo);
        }
    }

    if (!addedList.isEmpty()) {
        Q_EMIT deviceOnline(addedList);
    }
}

void DiscoverController::compatRemoveDeivce(const QString &ip)
{
    // 更新设备状态为离线状态
    auto oldinfo = findDeviceByIP(ip);
    if (oldinfo)
        d->onlineDeviceList.removeOne(oldinfo);
    Q_EMIT deviceOffline(ip);
}

void DiscoverController::startDiscover()
{
    if (!d->zeroConf)
        return;

    // 延迟,为了展示发现界面
    QTimer::singleShot(500, [this]() {
        // 兼容，获取发现列表，refresh则清除数据
        Q_EMIT startDiscoveryDevice();
        refresh();
    });
}
