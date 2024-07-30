/*
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "configurationproxy.h"

ConfigurationProxy::ConfigurationProxy(QObject *parent)
    : QObject(parent)
    , mConfiguration(&Configuration::self())
{
    connect(mConfiguration, &Configuration::airplaneModeEnabledChanged, this, &ConfigurationProxy::airplaneModeEnabledChanged);
    connect(mConfiguration, &Configuration::manageVirtualConnectionsChanged, this, &ConfigurationProxy::manageVirtualConnectionsChanged);
}

bool ConfigurationProxy::unlockModemOnDetection() const
{
    if (!mConfiguration) {
        return false;
    }
    return mConfiguration->unlockModemOnDetection();
}

void ConfigurationProxy::setUnlockModemOnDetection(bool unlock)
{
    if (mConfiguration) {
        mConfiguration->setUnlockModemOnDetection(unlock);
    }
}

bool ConfigurationProxy::manageVirtualConnections() const
{
    if (!mConfiguration) {
        return false;
    }
    return mConfiguration->manageVirtualConnections();
}

void ConfigurationProxy::setManageVirtualConnections(bool manage)
{
    if (mConfiguration) {
        mConfiguration->setManageVirtualConnections(manage);
    }
}

bool ConfigurationProxy::airplaneModeEnabled() const
{
    if (!mConfiguration) {
        return false;
    }
    return mConfiguration->airplaneModeEnabled();
}

void ConfigurationProxy::setAirplaneModeEnabled(bool enabled)
{
    if (mConfiguration) {
        mConfiguration->setAirplaneModeEnabled(enabled);
    }
}

QString ConfigurationProxy::hotspotName() const
{
    return mConfiguration->hotspotName();
}

void ConfigurationProxy::setHotspotName(const QString &name)
{
    if (mConfiguration) {
        mConfiguration->setHotspotName(name);
    }
}

QString ConfigurationProxy::hotspotPassword() const
{
    return mConfiguration->hotspotPassword();
}

void ConfigurationProxy::setHotspotPassword(const QString &password)
{
    if (mConfiguration) {
        mConfiguration->setHotspotPassword(password);
    }
}

QString ConfigurationProxy::hotspotConnectionPath() const
{
    return mConfiguration->hotspotConnectionPath();
}

void ConfigurationProxy::setHotspotConnectionPath(const QString &path)
{
    if (mConfiguration) {
        mConfiguration->setHotspotConnectionPath(path);
    }
}

bool ConfigurationProxy::showPasswordDialog() const
{
    if (!mConfiguration) {
        return false;
    }
    return mConfiguration->showPasswordDialog();
}

bool ConfigurationProxy::systemConnectionsByDefault() const
{
    if (!mConfiguration) {
        return false;
    }

    return mConfiguration->systemConnectionsByDefault();
}

#include "moc_configurationproxy.cpp"
