// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transferwrapper.h"
#include "networkutil.h"
#include "helper/transferhelper.h"
#include "discover/discovercontroller.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace cooperation_core;

TransferWrapper::TransferWrapper(QObject *parent)
    : CuteIPCService(parent)
{
    connect(DiscoverController::instance(), &DiscoverController::deviceOnline, this, &TransferWrapper::onDeviceOnline);
    connect(DiscoverController::instance(), &DiscoverController::deviceOffline, this, &TransferWrapper::onDeviceOffline);
    connect(DiscoverController::instance(), &DiscoverController::discoveryFinished, this, &TransferWrapper::onFinishedDiscovery);
}

TransferWrapper::~TransferWrapper()
{
}

TransferWrapper *TransferWrapper::instance()
{
    static TransferWrapper ins;
    return &ins;
}

void TransferWrapper::onRefreshDevice()
{
    DiscoverController::instance()->startDiscover();
}

void TransferWrapper::onSearchDevice(const QString &ip)
{
    NetworkUtil::instance()->trySearchDevice(ip);
}

void TransferWrapper::onSendFiles(const QString &ip, const QString &name, const QStringList &files)
{
    TransferHelper::instance()->sendFiles(ip, name, files);
}

void TransferWrapper::onDeviceOnline(const QList<DeviceInfoPointer> &infoList)
{
    QStringList devList;
    for (auto info : infoList) {
        auto infomap = info->toVariantMap();
        devList << variantMapToQString(infomap);
    }
    if (devList.isEmpty())
        return;

    if (devList.size() < 2) {
        Q_EMIT searched(devList.first());
    } else {
        Q_EMIT refreshed(devList);
    }
}

void TransferWrapper::onDeviceOffline(const QString &ip)
{
    Q_EMIT deviceChanged(false, ip);
}

void TransferWrapper::onFinishedDiscovery(bool hasFound)
{
    if (!hasFound)
        Q_EMIT searched("");
}


QString TransferWrapper::variantMapToQString(const QVariantMap &variantMap)
{
    QJsonObject jsonObject = QJsonObject::fromVariantMap(variantMap);
    QJsonDocument jsonDoc(jsonObject);
    return jsonDoc.toJson(QJsonDocument::Compact);
}
