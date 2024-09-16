// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AIRPLANMODECONTROLLER_H
#define AIRPLANMODECONTROLLER_H

#include <QObject>

#include <DSingleton>

#include <com_deepin_daemon_airplanemode.h>
#include <org_freedesktop_login1.h>

using DBusAirplaneMode = com::deepin::daemon::AirplaneMode;
using Login1Manager = org::freedesktop::login1::Manager;

class AirplaneModeController : public QObject, public Dtk::Core::DSingleton<AirplaneModeController>
{
    friend class Dtk::Core::DSingleton<AirplaneModeController>;

    Q_OBJECT
public:
    bool isEnabled() const;

public Q_SLOTS:
    void enable(bool enable);
    void toggle();

Q_SIGNALS:
    void enabledChanged(bool enabled);
    void prepareForSleep(bool sleep);

private:
    AirplaneModeController();
    ~AirplaneModeController();

private:
    DBusAirplaneMode *m_airplaneModeInter;
    Login1Manager *m_login1Manager;
};

#define AMC_PTR (&AirplaneModeController::ref())

#endif
