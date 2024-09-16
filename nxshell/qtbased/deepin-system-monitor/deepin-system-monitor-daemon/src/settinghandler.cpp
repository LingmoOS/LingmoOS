// SPDX-FileCopyrightText: 2011 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settinghandler.h"
#include "ddlog.h"
#include <DSettingsOption>

#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QVariant>
#include <QString>

using namespace DDLog;

// 警报数据，由产品定义
const static int MinAlarmCpuUsage = 30;
const static int MaxAlarmCpuUsage = 100;
const static int MinAlarmMemUsage = 30;
const static int MaxAlarmMemUsage = 100;
const static int MinAlarmInterval = 5;
const static int MaxAlarmInterval = 60;

SettingHandler::SettingHandler(QObject *parent)
    : QObject(parent), mSettings(nullptr), mBackend(nullptr)
{
    QString orgName = qApp->organizationName();
    if (orgName.isEmpty()) {
        orgName = "deepin";
    }

    //配置文件路径: ~/.config/deepin/deepin-system-monitor/protection.conf
    QString strConfigPath = QString("%1/%2/%3/protection.conf")
                                    .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                                    .arg(orgName)
                                    .arg("deepin-system-monitor");

    mBackend = new Dtk::Core::QSettingBackend(strConfigPath);
    mSettings = DSettings::fromJsonFile(":/resources/settings.json");

    auto opt = mSettings->option(AlarmCpuUsageOptionName);
    // 初始化有效值范围
    mValueRange[AlarmCpuUsageOptionName] = QPair<double, double>(MinAlarmCpuUsage, MaxAlarmCpuUsage);
    mValueRange[AlarmMemUsageOptionName] = QPair<double, double>(MinAlarmMemUsage, MaxAlarmMemUsage);
    mValueRange[AlarmIntervalOptionName] = QPair<double, double>(MinAlarmInterval, MaxAlarmInterval);

    auto keys = mValueRange.keys();

    foreach (auto it, keys) {
        qCDebug(app) << __FUNCTION__ << __LINE__ << "，key: " << it << ", range: " << mValueRange[it];
    }

    if (mSettings != nullptr) {
        mSettings->setBackend(mBackend);
    }
}

SettingHandler::~SettingHandler()
{
    if (mSettings != nullptr) {
        mSettings->deleteLater();
    }

    if (mBackend != nullptr) {
        mBackend->deleteLater();
    }
}

bool SettingHandler::isCompelted()
{
    return (mBackend != nullptr && mSettings != nullptr);
}

QVariant SettingHandler::getOptionValue(const QString key)
{
    if (isCompelted() && mSettings->keys().contains(key)) {
        return mSettings->getOption(key);
    } else {
        qCWarning(app) << __FUNCTION__ << __LINE__ << QString("can not find conf[%1]!").arg(key)
                   << ", compeletd:" << isCompelted()
                   << ", avalid keys:" << mSettings->keys();
    }

    return QVariant();
}

bool SettingHandler::changedOptionValue(const QString key, const QVariant value)
{
    if (isCompelted() && mSettings->keys().contains(key)) {
        auto opt = mSettings->option(key);
        opt->setValue(value);
        mSettings->sync();
        return true;
    } else {
        qCWarning(app) << __FUNCTION__ << __LINE__ << QString("change conf[%1,%2] fail !").arg(key).arg(value.toString())
                   << ", compeletd:" << isCompelted()
                   << ", avalid keys:" << mSettings->keys();
    }

    return false;
}

bool SettingHandler::isVaildValue(const QString key, const QVariant value)
{
    if (mValueRange.contains(key) && value.type() >= QVariant::Bool && value.type() <= QVariant::Double) {
        QPair<double, double> range = mValueRange[key];
        double valueD = value.toDouble();
        if (valueD < range.first || valueD > range.second) {
            return false;
        } else {
            return true;
        }
    }
    return true;
}

QPair<double, double> SettingHandler::getValueRange(const QString key)
{
    QPair<double, double> range(0, 0);
    if (mValueRange.contains(key)) {
        range = mValueRange[key];
    }
    return range;
}

QList<QString> SettingHandler::itemKeys()
{
    return mSettings->keys();
}
