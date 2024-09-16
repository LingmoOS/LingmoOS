// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingconfig.h"
#include <DConfig>

static Dtk::Core::DConfig *dConfig = nullptr;

SettingConfig *SettingConfig::instance()
{
    static SettingConfig inst;
    return &inst;
}

bool SettingConfig::reconnectIfIpConflicted() const
{
    return m_reconnectIfIpConflicted;
}

bool SettingConfig::enableConnectivity() const
{
    return m_enableConnectivity;
}

int SettingConfig::connectivityCheckInterval() const
{
    return m_connectivityCheckInterval;
}

QStringList SettingConfig::networkCheckerUrls() const
{
    return m_networkUrls;
}

bool SettingConfig::checkPortal() const
{
    return m_checkPortal;
}

bool SettingConfig::disableNetwork() const
{
    return m_disabledNetwork;
}

bool SettingConfig::enableAccountNetwork() const
{
    return m_enableAccountNetwork;
}

void SettingConfig::onValueChanged(const QString &key)
{
    if (key == "reconnectIfIpConflicted") {
        m_reconnectIfIpConflicted = dConfig->value(key).toBool();
    } else if (key == "enableConnectivity") {
        m_enableConnectivity = dConfig->value(key).toBool();
        enableConnectivityChanged(m_enableConnectivity);
    } else if (key == QString("ConnectivityCheckInterval")) {
        m_connectivityCheckInterval = dConfig->value("ConnectivityCheckInterval").toInt() * 1000;
        emit connectivityCheckIntervalChanged(m_connectivityCheckInterval);
    } else if (key == QString("NetworkCheckerUrls")) {
        m_networkUrls = dConfig->value("NetworkCheckerUrls").toStringList();
        emit checkUrlsChanged(m_networkUrls);
    } else if (key == QString("checkPortal")) {
        m_checkPortal = dConfig->value("checkPortal").toBool();
        emit checkPortalChanged(m_checkPortal);
    }
}

SettingConfig::SettingConfig(QObject *parent)
    : QObject(parent)
    , m_reconnectIfIpConflicted(false)
    , m_enableConnectivity(true)
    , m_connectivityCheckInterval(30000)
    , m_checkPortal(false)
    , m_disabledNetwork(false)
    , m_enableAccountNetwork(false)
{
    if (!dConfig)
        dConfig = Dtk::Core::DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.network");

    if (dConfig && dConfig->isValid()) {
        connect(dConfig, &Dtk::Core::DConfig::valueChanged, this, &SettingConfig::onValueChanged);

        QStringList keys = dConfig->keyList();
        if (keys.contains("reconnectIfIpConflicted"))
            m_reconnectIfIpConflicted = dConfig->value("reconnectIfIpConflicted").toBool();

        if (keys.contains("enableConnectivity"))
            m_enableConnectivity = dConfig->value("enableConnectivity").toBool();

        if (keys.contains("ConnectivityCheckInterval"))
            m_connectivityCheckInterval = dConfig->value("ConnectivityCheckInterval").toInt();

        if (keys.contains("NetworkCheckerUrls"))
            m_networkUrls = dConfig->value("NetworkCheckerUrls").toStringList();

        if (keys.contains("checkPortal"))
            m_checkPortal = dConfig->value("checkPortal").toBool();

        if (keys.contains("disabledNetwork"))
            m_disabledNetwork = dConfig->value("disabledNetwork").toBool();

        if (keys.contains("enableAccountNetwork"))
            m_enableAccountNetwork = dConfig->value("enableAccountNetwork").toBool();
    }
}
