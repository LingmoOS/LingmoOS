// SPDX-FileCopyrightText: 2011 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemmonitorservice.h"
#include "ddlog.h"
#include <DSettingsOption>
#include <QDBusInterface>

#include <QDebug>
#include <QProcess>
#include <QDBusVariant>
#include <QFile>
#include <QDBusConnectionInterface>
#include <QDateTime>

using namespace DDLog;

// 打印DBus调用者信息
#define PrintDBusCaller()                                                                                   \
    {                                                                                                       \
        if (calledFromDBus()) {                                                                             \
            QDBusConnection conn = connection();                                                            \
            QDBusMessage msg = message();                                                                   \
            pid_t callerPid = static_cast<pid_t>(conn.interface()->servicePid(msg.service()).value());      \
            qCDebug(app) << "DBus service caller:" << conn.interface()->serviceOwner(msg.service()).value() \
                         << ",Uid:" << conn.interface()->serviceUid(msg.service()).value()                  \
                         << ",Pid:" << callerPid                                                            \
                         << ",Process name:" << getNameByPid(callerPid);                                    \
        }                                                                                                   \
    }

// 通过PID获取进程名称
QString getNameByPid(pid_t pid)
{
    QString name;
    QFile file(QString("/proc/%1/cmdline").arg(pid));
    if (file.open(QFile::ReadOnly)) {
        name = file.readLine();
        file.close();
    }
    return name;
}

#define InitAlarmCpuUsage 90
#define InitAlarmMemUsage 90
#define InitAlarmInterval 10
#define InitAlarmOn false
#define MonitorTimeOut 1000
#define AlarmMessageTimeOut 10000

SystemMonitorService::SystemMonitorService(const char *name, QObject *parent)
    : QObject(parent), mProtectionStatus(InitAlarmOn), mAlarmInterval(InitAlarmInterval), mAlarmCpuUsage(InitAlarmCpuUsage), mAlarmMemoryUsage(InitAlarmMemUsage), mCpuUsage(0), mMemoryUsage(0), mMoniterTimer(this)
      //    , mLastAlarmTimeStamp(0)
      ,
      mSettings(this),
      mCpu(this),
      mMem(this)
{
    if (mSettings.isCompelted()) {
        mProtectionStatus = mSettings.getOptionValue(AlarmStatusOptionName).toBool();
        mAlarmCpuUsage = mSettings.getOptionValue(AlarmCpuUsageOptionName).toInt();
        mAlarmMemoryUsage = mSettings.getOptionValue(AlarmMemUsageOptionName).toInt();
        mAlarmInterval = mSettings.getOptionValue(AlarmIntervalOptionName).toInt();
        mLastAlarmTimeStamp = mSettings.getOptionValue(AlarmLastTimeOptionName).toLongLong();
    }

    // 初始化Cpu占用率
    mCpuUsage = static_cast<int>(mCpu.updateSystemCpuUsage());
    // 初始化Memory占用率
    mMemoryUsage = static_cast<int>(mMem.updateSystemMemoryUsage());

    // 从配置文件，初始化： mProtectionStatus mAlarmInterval mAlarmCpuUsage mAlarmMemoryUsage
    mMoniterTimer.setInterval(MonitorTimeOut);
    connect(&mMoniterTimer, &QTimer::timeout, this, &SystemMonitorService::onMonitorTimeout);

    // 启动监测定时器
    mMoniterTimer.start();

    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SessionBus, QString(name))
            .registerObject("/org/deepin/SystemMonitorDaemon", this, opts);
}

SystemMonitorService::~SystemMonitorService()
{
}

bool SystemMonitorService::getSystemProtectionStatus()
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Get Protection Status:" << mProtectionStatus;
    return mProtectionStatus;
}

void SystemMonitorService::setSystemProtectionStatus(bool status)
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Set Protection Status:" << status;

    if (mProtectionStatus != status) {
        mProtectionStatus = status;
        // 更改设置文件
        mSettings.changedOptionValue(AlarmStatusOptionName, mProtectionStatus);
        // 监测设置变更，DBus信号
        emit alarmItemChanged(AlarmStatusOptionName, QDBusVariant(mProtectionStatus));
    }
}

int SystemMonitorService::getCpuUsage()
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Get Cpu Usage:" << mCpuUsage;
    return mCpuUsage;
}

int SystemMonitorService::getMemoryUsage()
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Get Memory Usage:" << mMemoryUsage;
    return mMemoryUsage;
}

int SystemMonitorService::getAlarmMsgInterval()
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Get Alarm Msg Interval:" << mAlarmInterval;
    return mAlarmInterval;
}

void SystemMonitorService::setAlarmMsgInterval(int interval)
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Set Alarm Msg Interval:" << interval;

    // 根据需求
    if (mSettings.isVaildValue(AlarmIntervalOptionName, interval)) {
        mAlarmInterval = interval;
        // 更改设置文件
        mSettings.changedOptionValue(AlarmIntervalOptionName, mAlarmInterval);
        // 监测设置变更，DBus信号
        emit alarmItemChanged(AlarmIntervalOptionName, QDBusVariant(mAlarmInterval));
    } else {
        QPair<double, double> range = mSettings.getValueRange(AlarmIntervalOptionName);
        sendErrorReply(QDBusError::NotSupported, QString("invalid value! value range[%1,%2]").arg(range.first).arg(range.second));
    }
}

qint64 SystemMonitorService::getAlaramLastTimeInterval()
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Get Alarm Last Time:" << mLastAlarmTimeStamp;
    return mLastAlarmTimeStamp;
}

void SystemMonitorService::setAlaramLastTimeInterval(qint64 lastTime)
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Set Alarm Last Time:" << lastTime;

    // 根据需求
    if (mSettings.isVaildValue(AlarmLastTimeOptionName, lastTime)) {
        mLastAlarmTimeStamp = lastTime;
        // 更改设置文件
        mSettings.changedOptionValue(AlarmLastTimeOptionName, mLastAlarmTimeStamp);
        // 监测设置变更，DBus信号
        emit alarmItemChanged(AlarmLastTimeOptionName, QDBusVariant(mLastAlarmTimeStamp));
    } else {
        QPair<double, double> range = mSettings.getValueRange(AlarmLastTimeOptionName);
        sendErrorReply(QDBusError::NotSupported, QString("invalid value! value range[%1,%2]").arg(range.first).arg(range.second));
    }
}

int SystemMonitorService::getAlarmUsageOfCpu()
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Get Alarm Usage Of Cpu:" << mAlarmCpuUsage;
    return mAlarmCpuUsage;
}

void SystemMonitorService::setAlarmUsageOfCpu(int usage)
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Set Alarm Usage Of Cpu:" << usage;

    // 根据需求
    if (mSettings.isVaildValue(AlarmCpuUsageOptionName, usage)) {
        mAlarmCpuUsage = usage;
        // 更改设置文件
        mSettings.changedOptionValue(AlarmCpuUsageOptionName, mAlarmCpuUsage);
        // 监测设置变更，DBus信号
        emit alarmItemChanged(AlarmCpuUsageOptionName, QDBusVariant(mAlarmCpuUsage));
    } else {
        QPair<double, double> range = mSettings.getValueRange(AlarmCpuUsageOptionName);
        sendErrorReply(QDBusError::NotSupported, QString("invalid value! value range[%1,%2]").arg(range.first).arg(range.second));
    }
}

int SystemMonitorService::getAlarmUsageOfMemory()
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Get Alarm Usage Of Memory:" << mAlarmMemoryUsage;
    return mAlarmMemoryUsage;
}

void SystemMonitorService::setAlarmUsageOfMemory(int usage)
{
    PrintDBusCaller()
                    qCDebug(app)
            << __FUNCTION__ << __LINE__ << " Set Alarm Usage Of Memory:" << usage;

    // 根据需求
    if (mSettings.isVaildValue(AlarmMemUsageOptionName, usage)) {
        mAlarmMemoryUsage = usage;
        // 更改设置文件
        mSettings.changedOptionValue(AlarmMemUsageOptionName, mAlarmMemoryUsage);
        // 监测设置变更，DBus信号
        emit alarmItemChanged(AlarmMemUsageOptionName, QDBusVariant(mAlarmMemoryUsage));
    } else {
        QPair<double, double> range = mSettings.getValueRange(AlarmMemUsageOptionName);
        sendErrorReply(QDBusError::NotSupported, QString("invalid value! value range[%1,%2]").arg(range.first).arg(range.second));
    }
}

void SystemMonitorService::showDeepinSystemMoniter()
{
    PrintDBusCaller()

            QString cmd("gdbus call -e -d  com.deepin.SystemMonitorServer -o /com/deepin/SystemMonitorServer -m com.deepin.SystemMonitorServer.showDeepinSystemMoniter");
    QTimer::singleShot(100, this, [=]() { QProcess::startDetached(cmd); });
}

void SystemMonitorService::changeAlarmItem(const QString &item, const QDBusVariant &value)
{
    qCDebug(app) << __FUNCTION__ << __LINE__ << ", item:" << item << ", value:" << value.variant();
    if (mSettings.itemKeys().contains(item) && mSettings.isVaildValue(item, value.variant())) {
        if (mSettings.isVaildValue(item, value.variant())) {
            if (item == AlarmStatusOptionName) {
                mProtectionStatus = value.variant().toBool();
            } else if (item == AlarmCpuUsageOptionName) {
                mAlarmCpuUsage = value.variant().toInt();
            } else if (item == AlarmMemUsageOptionName) {
                mAlarmMemoryUsage = value.variant().toInt();
            } else if (item == AlarmIntervalOptionName) {
                mAlarmInterval = value.variant().toInt();
            }
            // 更改设置文件
            mSettings.changedOptionValue(item, value.variant());
        } else {
            QPair<double, double> range = mSettings.getValueRange(item);
            sendErrorReply(QDBusError::NotSupported, QString("invalid value! value range[%1,%2]").arg(range.first).arg(range.second));
        }
    } else {
        sendErrorReply(QDBusError::NotSupported, QString("invalid item(%1) !").arg(item));
    }
}

bool SystemMonitorService::checkCpuAlarm()
{
    qint64 curTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 diffTime = curTimeStamp - mLastAlarmTimeStamp;
    qint64 timeGap = 1000 * 60 * mAlarmInterval;

    if (mCpuUsage >= mAlarmCpuUsage && diffTime >= timeGap) {
        mLastAlarmTimeStamp = curTimeStamp;
        QString cmd = QString("gdbus call -e -d  com.deepin.SystemMonitorServer -o /com/deepin/SystemMonitorServer -m com.deepin.SystemMonitorServer.showCpuAlarmNotify \"%1\" ").arg(QString::number(mCpuUsage));
        QTimer::singleShot(100, this, [=]() { QProcess::startDetached(cmd); });
    }

    return false;
}

bool SystemMonitorService::checkMemoryAlarm()
{
    qint64 curTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 diffTime = curTimeStamp - mLastAlarmTimeStamp;
    qint64 timeGap = 1000 * 60 * mAlarmInterval;

    if (mMemoryUsage >= mAlarmMemoryUsage && diffTime > timeGap) {
        mLastAlarmTimeStamp = curTimeStamp;
        QString cmd = QString("gdbus call -e -d  com.deepin.SystemMonitorServer -o /com/deepin/SystemMonitorServer -m com.deepin.SystemMonitorServer.showMemoryAlarmNotify \"%1\" ").arg(QString::number(mMemoryUsage));
        QTimer::singleShot(100, this, [=]() { QProcess::startDetached(cmd); });
    }

    return false;
}

void SystemMonitorService::onMonitorTimeout()
{
    // 获取CPU和内存占用
    mCpuUsage = static_cast<int>(mCpu.updateSystemCpuUsage());
    mMemoryUsage = static_cast<int>(mMem.updateSystemMemoryUsage());

    // 进行警报检测
    if (mProtectionStatus) {
        checkCpuAlarm();
        checkMemoryAlarm();
    }
}
