// SPDX-FileCopyrightText: 2011 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGHANDLER_H
#define SETTINGHANDLER_H

#include <DSettings>
#include <qsettingbackend.h>

#include <QObject>
#include <QMap>
#include <QPair>

DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

#define AlarmStatusOptionName   "setting.systemprotection.alarm_switch"
#define AlarmCpuUsageOptionName "setting.systemprotection.alarm_cpu_usage"
#define AlarmMemUsageOptionName "setting.systemprotection.alarm_memory_usage"
#define AlarmIntervalOptionName "setting.systemprotection.alarm_interval"
#define AlarmLastTimeOptionName "setting.systemprotection.alarm_last_time"

class SettingHandler : public QObject
{
    Q_OBJECT
public:
    explicit SettingHandler(QObject *parent = nullptr);
    ~SettingHandler();

signals:
public slots:

public:
    /*!
     * 数据加载完成
     */
    bool isCompelted();
    /*!
     * 获取设置项值
     */
    QVariant getOptionValue(const QString key);
    /*!
     * 设定设置项值
     */
    bool changedOptionValue(const QString key, const QVariant value);
    /*!
     * 判断此值是否是有效值
     */
    bool isVaildValue(const QString key, const QVariant value);
    /*!
     * 获取有效值范围
     */
    QPair<double, double> getValueRange(const QString key);
    QList<QString> itemKeys();

private:
    DSettings *mSettings;
    Dtk::Core::QSettingBackend *mBackend;
    QMap<QString, QPair<double, double>> mValueRange;
};

#endif // SETTINGHANDLER_H
