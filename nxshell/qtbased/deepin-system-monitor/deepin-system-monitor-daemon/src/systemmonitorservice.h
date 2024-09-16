// SPDX-FileCopyrightText: 2011 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMMONITORSERVICE_H
#define SYSTEMMONITORSERVICE_H

#include "settinghandler.h"
#include "cpuprofile.h"
#include "memoryprofile.h"

#include <DSettings>
#include <qsettingbackend.h>

#include <QObject>
#include <QDBusContext>
#include <QDBusVariant>
#include <QDBusAbstractAdaptor>
#include <QTimer>

DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

class SystemMonitorService : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.SystemMonitorDaemon")
public:
    explicit SystemMonitorService(const char *name, QObject *parent = nullptr);
    ~SystemMonitorService();

public slots:
    /*!
     * DBus Adaptor接口: 获取检测开关状态
     */
    Q_SCRIPTABLE bool getSystemProtectionStatus();
    /*!
     * DBus Adaptor接口: 设置检测开关状态
     */
    Q_SCRIPTABLE void setSystemProtectionStatus(bool status);
    /*!
     * DBus Adaptor接口: 获取当前Cpu占用率
     */
    Q_SCRIPTABLE int getCpuUsage();
    /*!
     * DBus Adaptor接口: 获取当前Memory占用率
     */
    Q_SCRIPTABLE int getMemoryUsage();
    /*!
     * DBus Adaptor接口: 获取当前报警信息间隔时间(minute)
     */
    Q_SCRIPTABLE int getAlarmMsgInterval();
    /*!
     * DBus Adaptor接口: 设置当前报警信息间隔时间(minute)
     */
    Q_SCRIPTABLE void setAlarmMsgInterval(int interval);
    /*!
     * DBus Adaptor接口: 获取当前监测报警Cpu占用率(%)
     */
    Q_SCRIPTABLE int getAlarmUsageOfCpu();
    /*!
     * DBus Adaptor接口: 设置当前监测报警Cpu占用率(%)
     */
    Q_SCRIPTABLE void setAlarmUsageOfCpu(int usage);
    /*!
     * DBus Adaptor接口: 获取当前监测报警Memory占用率(%)
     */
    Q_SCRIPTABLE int getAlarmUsageOfMemory();
    /*!
     * DBus Adaptor接口: 设置当前监测报警Memory占用率(%)
     */
    Q_SCRIPTABLE void setAlarmUsageOfMemory(int usage);
    /*!
     * DBus Adaptor接口: 显示系统监视器主页面
     */
    Q_SCRIPTABLE void showDeepinSystemMoniter();
    /*!
     * DBus Adaptor接口: 修改监控设置
     */
    Q_SCRIPTABLE void changeAlarmItem(const QString &item, const QDBusVariant &value);
    /*!
     * \brief setAlaramLastTimeInterval 设置上次告警时间
     * \param lastTime 设置的参数值
     */
    Q_SCRIPTABLE void setAlaramLastTimeInterval(qint64 lastTime);

private:
    /*!
     * 检查是否触发Cpu报警
     */
    bool checkCpuAlarm();
    /*!
     * 检查是否触发Memory报警
     */
    bool checkMemoryAlarm();

    /*!
     * \brief getAlaramLastTimeInterval 获取上次告警时间
     * \return 上次告警时间
     */
    qint64 getAlaramLastTimeInterval();

signals:
    /*!
     * 监测设置变更信号
     */
    void alarmItemChanged(const QString &item, const QDBusVariant &value);

public slots:
    /*!
     * 监测由此计时器槽处理
     */
    void onMonitorTimeout();

private:
    /*!
     * setting值的副本
     */
    bool mProtectionStatus;
    int mAlarmInterval;
    int mAlarmCpuUsage;
    int mAlarmMemoryUsage;
    /*!
     * 当前CPU和内存占用率(%)
     */
    int mCpuUsage;
    int mMemoryUsage;
    /*!
     * 监测计时器及时间戳
     */
    QTimer mMoniterTimer;
    qint64 mLastAlarmTimeStamp;
    /*!
     * 设置数据类
     */
    SettingHandler mSettings;
    /*!
     * Cpu数据获取类
     */
    CpuProfile mCpu;
    /*!
     * Memory数据获取类
     */
    MemoryProfile mMem;
};

#endif // SYSTEMMONITORSERVICE_H
