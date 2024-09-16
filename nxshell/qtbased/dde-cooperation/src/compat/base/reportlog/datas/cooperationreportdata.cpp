// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationreportdata.h"

#include <DSysInfo>

#include <QDateTime>

using namespace deepin_cross;
DCORE_USE_NAMESPACE

static QVariantMap mergeCommonAttributes(const QVariantMap &args)
{
    QVariantMap map = args;

    if (DSysInfo::isDeepin()) {
        map.insert("systemVersion", DSysInfo::uosEditionName());
        map.insert("versionNumber", DSysInfo::minorVersion());
    }
    map.insert("sysTime", QDateTime::currentDateTime().toString("yyyy/MM/dd"));
    map.insert("machineID", QSysInfo::machineUniqueId());

    return map;
}

QString StatusReportData::type() const
{
    return "CooperationStatus";
}

QJsonObject StatusReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap data = mergeCommonAttributes(args);
    data.insert("tid", 1000800000);
    return QJsonObject::fromVariantMap(data);
}

QString FileDeliveryReportData::type() const
{
    return "FileDelivery";
}

QJsonObject FileDeliveryReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap data = mergeCommonAttributes(args);
    data.insert("tid", 1000800001);
    return QJsonObject::fromVariantMap(data);
}

QString ConnectionReportData::type() const
{
    return "ConnectionInfo";
}

QJsonObject ConnectionReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap data = mergeCommonAttributes(args);
    data.insert("tid", 1000800002);
    return QJsonObject::fromVariantMap(data);
}
