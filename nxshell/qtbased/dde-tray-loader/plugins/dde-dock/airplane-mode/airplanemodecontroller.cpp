// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "airplanemodecontroller.h"

AirplaneModeController::AirplaneModeController()
    :m_airplaneModeInter(new DBusAirplaneMode("com.deepin.daemon.AirplaneMode",
                                               "/com/deepin/daemon/AirplaneMode",
                                               QDBusConnection::systemBus(),
                                               this))
    , m_login1Manager(new Login1Manager("org.freedesktop.login1",
                                        "/org/freedesktop/login1",
                                        QDBusConnection::systemBus(),
                                        this))
{
    connect(m_airplaneModeInter, &DBusAirplaneMode::EnabledChanged, this, &AirplaneModeController::enabledChanged);
    connect(m_login1Manager, &Login1Manager::PrepareForSleep, this, &AirplaneModeController::prepareForSleep);
}

AirplaneModeController::~AirplaneModeController()
{

}

void AirplaneModeController::enable(bool enable)
{
    m_airplaneModeInter->EnableQueued(enable);
}

void AirplaneModeController::toggle()
{
    enable(!isEnabled());
}

bool AirplaneModeController::isEnabled() const
{
    return m_airplaneModeInter->enabled();
}
