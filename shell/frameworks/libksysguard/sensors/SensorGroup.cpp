/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorGroup_p.h"

#include <KLocalizedString>
#include <QDebug>
#include <QRegularExpression>

namespace KSysGuard
{
SensorGroup::SensorGroup()
{
    retranslate();
}

SensorGroup::~SensorGroup()
{
}

void SensorGroup::retranslate()
{
    m_sensorNames.clear();
    m_segmentNames.clear();

    m_sensorNames[QStringLiteral("cpu/cpu\\d+/usage")] = i18nc("Total load sensor of all cores", "[Group] Total Usage");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/user")] = i18nc("All cores user load sensors", "[Group] User Usage");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/system")] = i18nc("All cores user system sensors", "[Group] System Usage");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/wait")] = i18nc("All cores wait load sensors", "[Group] Wait Usage");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/frequency")] = i18nc("All cores clock frequency sensors", "[Group] Clock Frequency");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/temperature")] = i18nc("All cores temperature sensors", "[Group] Temperature");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/name")] = i18nc("All cores names", "[Group] Name");

    m_sensorNames[QStringLiteral("partitions/(?!all).*/usedspace")] = i18n("[Group] Used");
    m_sensorNames[QStringLiteral("partitions/(?!all).*/freespace")] = i18n("[Group] Available");
    m_sensorNames[QStringLiteral("partitions/(?!all).*/filllevel")] = i18n("[Group] Percentage Used");
    m_sensorNames[QStringLiteral("partitions/(?!all).*/total")] = i18n("[Group] Total Size");

    m_sensorNames[QStringLiteral("network/(?!all).*/network")] = i18n("[Group] Network Name");
    m_sensorNames[QStringLiteral("network/(?!all).*/ipv4address")] = i18n("[Group] IPv4 Address");
    m_sensorNames[QStringLiteral("network/(?!all).*/ipv6address")] = i18n("[Group] IPv6 Address");
    m_sensorNames[QStringLiteral("network/(?!all).*/signal")] = i18n("[Group] Signal Strength");
    m_sensorNames[QStringLiteral("network/(?!all).*/download")] = i18n("[Group] Download Rate");
    m_sensorNames[QStringLiteral("network/(?!all).*/downloadBits")] = i18n("[Group] Download Rate");
    m_sensorNames[QStringLiteral("network/(?!all).*/totalDownload")] = i18n("[Group] Total Downloaded");
    m_sensorNames[QStringLiteral("network/(?!all).*/upload")] = i18n("[Group] Upload Rate");
    m_sensorNames[QStringLiteral("network/(?!all).*/uploadBits")] = i18n("[Group] Upload Rate");
    m_sensorNames[QStringLiteral("network/(?!all).*/totalUpload")] = i18n("[Group] Total Uploaded");

    m_sensorNames[QStringLiteral("disk/(?!all).*/name")] = i18n("[Group] Name");
    m_sensorNames[QStringLiteral("disk/(?!all).*/total")] = i18n("[Group] Total Space");
    m_sensorNames[QStringLiteral("disk/(?!all).*/used")] = i18n("[Group] Used Space");
    m_sensorNames[QStringLiteral("disk/(?!all).*/free")] = i18n("[Group] Free Space");
    m_sensorNames[QStringLiteral("disk/(?!all).*/read")] = i18n("[Group] Read Rate");
    m_sensorNames[QStringLiteral("disk/(?!all).*/write")] = i18n("[Group] Write Rate");
    m_sensorNames[QStringLiteral("disk/(?!all).*/usedPercent")] = i18n("[Group] Percentage Used");
    m_sensorNames[QStringLiteral("disk/(?!all).*/freePercent")] = i18n("[Group] Percentage Free");

    m_segmentNames[QLatin1String("cpu\\d+")] = i18n("[Group] CPU");
    m_segmentNames[QLatin1String("disk/(?!all).*")] = i18n("[Group] Disk");
    m_segmentNames[QLatin1String("(?!all).*")] = i18n("[Group]");
}

QString SensorGroup::groupRegexForId(const QString &key)
{
    QRegularExpression cpuExpr(QStringLiteral("cpu/cpu\\d+/(.*)"));
    QRegularExpression netExpr(QStringLiteral("network/(?!all).*/(.*)$"));
    QRegularExpression partitionsExpr(QStringLiteral("partitions/(?!all).*/(.*)$"));
    QRegularExpression diskExpr(QStringLiteral("disk/(?!all).*/(.*)"));

    if (key.contains(cpuExpr)) {
        QString expr = key;
        return expr.replace(cpuExpr, QStringLiteral("cpu/cpu\\d+/\\1"));

    } else if (key.contains(netExpr)) {
        QString expr = key;
        return expr.replace(netExpr, QStringLiteral("network/(?!all).*/\\1"));

    } else if (key.contains(partitionsExpr)) {
        QString expr = key;
        return expr.replace(partitionsExpr, QStringLiteral("partitions/(?!all).*/\\1"));

    } else if (key.contains(diskExpr)) {
        QString expr = key;
        return expr.replace(diskExpr, QStringLiteral("disk/(?!all).*/\\1"));
    }

    return QString();
}

QString SensorGroup::sensorNameForRegEx(const QString &expr)
{
    if (m_sensorNames.contains(expr)) {
        return m_sensorNames.value(expr);
    }

    return expr;
}

QString SensorGroup::segmentNameForRegEx(const QString &expr)
{
    if (m_segmentNames.contains(expr)) {
        return m_segmentNames.value(expr);
    }

    return expr;
}

} // namespace KSysGuard
