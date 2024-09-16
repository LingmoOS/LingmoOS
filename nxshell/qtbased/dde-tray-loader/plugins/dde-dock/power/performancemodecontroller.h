// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PERFORMANCEMODECONTROLLER_H
#define PERFORMANCEMODECONTROLLER_H

#include <QObject>
#include <QGSettings>
#include <QVariant>

#include <DSingleton>

#include <com_deepin_system_systempower.h>

#define PERFORMANCE "performance" // 高性能模式
#define BALANCE     "balance"     // 平衡模式
#define POWERSAVE   "powersave"   // 节能模式
#define BALANCEPERFORMANCE "balance_performance" // 性能模式

using SysPowerInter = com::deepin::system::Power;

class PerformanceModeController : public QObject, public Dtk::Core::DSingleton<PerformanceModeController>
{
    friend Dtk::Core::DSingleton<PerformanceModeController>;

    Q_OBJECT
public:
    bool highPerformanceSupported() const { return m_highPerformanceSupported; }
    bool balanceSupported() const { return m_balanceSupported; }
    bool powerSaveSupported() const { return m_powerSaveSupported; }
    QList<QPair<QString, QString>> modeList() const { return m_modeList; }

    void setPowerMode(const QString &mode)
    {
        m_sysPowerInter->SetMode(mode);
    }
    QString getCurrentPowerMode() const { return m_currentMode; }

Q_SIGNALS:
    void powerModeChanged(const QString &mode);
    void highPerformanceSupportChanged(bool value);

private:
    PerformanceModeController()
        : QObject(nullptr)
        , m_highPerformanceSupported(false)
        , m_balanceSupported(false)
        , m_powerSaveSupported(false)
        , m_currentMode(QString())
        , m_sysPowerInter(new SysPowerInter("com.deepin.system.Power", "/com/deepin/system/Power", QDBusConnection::systemBus(), this))
    {
        m_sysPowerInter->setSync(false);
        const QList<QPair<QString, QString>> powerModeList = {
            QPair<QString, QString>(BALANCE, tr("Balanced")),
            QPair<QString, QString>(BALANCEPERFORMANCE, tr("Balance Performance")),
            QPair<QString, QString>(PERFORMANCE, tr("High Performance")),
            QPair<QString, QString>(POWERSAVE, tr("Power Saver"))
        };
        const QMap<QString, QString>powerModeProperty = {
            {BALANCE, "IsBalanceSupported"},
            {BALANCEPERFORMANCE, "IsBalancePerformanceSupported"},
            {PERFORMANCE, "IsHighPerformanceSupported"},
            {POWERSAVE, "IsPowerSaveSupported"}
        };

        QDBusInterface interface("com.deepin.system.Power",
                                 "/com/deepin/system/Power",
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());

        auto getPowerModeProperty = [](QDBusInterface &interface, QString path, QString property)->bool {
            QDBusMessage reply = interface.call("Get", path, property);
            QList<QVariant> outArgs = reply.arguments();
            if (outArgs.length() > 0) {
                return  outArgs.at(0).value<QDBusVariant>().variant().toBool();
            }
            return false;
        };
        for (const auto &pair : powerModeList) {
            bool powerModeData = getPowerModeProperty(interface, "com.deepin.system.Power", powerModeProperty.value(pair.first));
            if (powerModeData) {
                m_modeList.append(pair);
            }
            if (pair.first == PERFORMANCE) {
                m_highPerformanceSupported = powerModeData;
            } else if (pair.first == BALANCE) {
                m_balanceSupported = powerModeData;
            } else if (pair.first == POWERSAVE) {
                m_powerSaveSupported = powerModeData;
            }
        }

        QDBusMessage reply = interface.call("Get", "com.deepin.system.Power", "Mode");
        QList<QVariant> outArgs = reply.arguments();
        if (outArgs.length() > 0) {
            m_currentMode = outArgs.at(0).value<QDBusVariant>().variant().toString();
        }

        qDebug()  << " currentMode data : " << m_currentMode;

        connect(m_sysPowerInter, &SysPowerInter::ModeChanged, this, [this](const QString &mode) {
            qDebug()  << " system power inter mode changed  : " <<  mode << m_currentMode;
            m_currentMode = mode;
            Q_EMIT powerModeChanged(m_currentMode);
        });
        connect(m_sysPowerInter, &SysPowerInter::IsHighPerformanceSupportedChanged, this, [this](bool value) {
            m_highPerformanceSupported = value;
            Q_EMIT highPerformanceSupportChanged(value);
        });
    }

    ~PerformanceModeController() {}

private:
    bool m_highPerformanceSupported;
    bool m_balanceSupported;
    bool m_powerSaveSupported;
    QList<QPair<QString, QString>> m_modeList;
    QString m_currentMode;

    SysPowerInter *m_sysPowerInter;
};

#endif
