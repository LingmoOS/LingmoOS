// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "monitor.h"
#include "brightnessmodel.h"

#include <QObject>
#include <QGSettings>
#include <QVariant>

#include <DSingleton>

#include <com_deepin_daemon_display.h>

using DisplayInter = com::deepin::daemon::Display;

class BrightnessController : public QObject, public Dtk::Core::DSingleton<BrightnessController>
{
    friend Dtk::Core::DSingleton<BrightnessController>;

    Q_OBJECT

Q_SIGNALS:
    void supportBrightnessChanged(const bool support);

public Q_SLOTS:
    void setMonitorBrightness(Monitor *mon, const double brightness);
    void init();
    bool supportBrightness() const { return m_supportBrightness; }

private Q_SLOTS:
    void onGSettingsChanged(const QString &key);
    void onMonitorListChanged(const QList<QDBusObjectPath> &mons);
    void onMonitorsBrightnessChanged(const BrightnessMap &brightness);

private:
    explicit BrightnessController(QObject *parent = 0);
    ~BrightnessController();

    void monitorAdded(const QString &path);
    void monitorRemoved(const QString &path);
    void handleSetBrightnessRequest();
    void updateSupportBrightnessState();

private:
    DisplayInter m_displayInter;
    QDBusInterface *m_displayDBusInter;
    QGSettings *m_gSettings;
    QMap<Monitor *, MonitorInter *> m_monitors;
    bool m_supportBrightness;

    struct {
        QMutex  m_brightnessMutex;
        bool    m_hasPendingRequest = false;
        bool    m_hasWaitingRequest = false;
        double  m_brightnessValue = 0;
        QString m_monitorName;
    }m_brightness;
};
