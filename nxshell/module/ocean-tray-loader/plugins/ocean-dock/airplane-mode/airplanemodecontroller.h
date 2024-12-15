// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AIRPLANMODECONTROLLER_H
#define AIRPLANMODECONTROLLER_H

#include <QObject>

#include <DSingleton>

#include "airplanemode1interface.h"
#include "login1interface.h"

using DBusAirplaneMode = org::lingmo::ocean::AirplaneMode1;
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
