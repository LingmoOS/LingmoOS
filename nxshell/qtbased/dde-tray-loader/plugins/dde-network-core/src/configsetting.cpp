// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configsetting.h"

#include <DConfig>

#include <QCoreApplication>
#include <QDebug>

using namespace dde::network;

static Dtk::Core::DConfig *dConfig = nullptr;

ConfigSetting::ConfigSetting(QObject *parent)
    : QObject(parent)
    , m_alwaysFromNM(false)
    , m_loadServiceFromNM(false)
    , m_enableConnectivity(false)
    , m_checkPortal(false)
    , m_supportCertifiedEscape(false)
    , m_showUnAuthorizeSwitch(true)
    , m_connectivityCheckInterval(30000)
    , m_wirelessScanInterval(10000)
    , m_wpaEapAuthen("peap")
    , m_wpaEapAuthmethod("gtc")
    , m_networkAirplaneMode(false)
    , m_enableAccountNetwork(false)
    , m_enableEapInput(false)
    , m_dontSetIpIfConflict(false)
    , m_showBrowserLink(false)
    , m_browserUrl("https://www.uniontech.com")
{
    QStringList keys;
    if (!dConfig)
        dConfig = Dtk::Core::DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.network");

    if (dConfig && dConfig->isValid()) {
        connect(dConfig, &Dtk::Core::DConfig::valueChanged, this, &ConfigSetting::onValueChanged);

        keys = dConfig->keyList();
        if (keys.contains("LoadServiceFromNM"))
            m_loadServiceFromNM = dConfig->value("LoadServiceFromNM").toBool();
        if (keys.contains("networkAirplaneMode"))
            m_networkAirplaneMode = dConfig->value("networkAirplaneMode").toBool();
        if (keys.contains("enableAccountNetwork"))
            m_enableAccountNetwork = dConfig->value("enableAccountNetwork").toBool();
        if (keys.contains("dontSetIpIfConflict"))
            m_dontSetIpIfConflict = dConfig->value("dontSetIpIfConflict").toBool();
        if (keys.contains("wpaEapAuthmethod"))
            m_wpaEapAuthmethod = dConfig->value("wpaEapAuthmethod").toString();
    }
    for (auto &&key : keys) {
        onValueChanged(key);
    }
}

ConfigSetting::~ConfigSetting() = default;

void ConfigSetting::onValueChanged(const QString &key)
{
    if (key == QString("NetworkCheckerUrls")) {
        m_networkUrls = dConfig->value("NetworkCheckerUrls").toStringList();
        emit checkUrlsChanged(m_networkUrls);
    } else if (key == QString("enableConnectivity")) {
        m_enableConnectivity = dConfig->value("enableConnectivity").toBool();
        emit enableConnectivityChanged(m_enableConnectivity);
    } else if (key == QString("checkPortal")) {
        m_checkPortal = dConfig->value("checkPortal").toBool();
        emit checkPortalChanged(m_checkPortal);
    } else if (key == QString("ConnectivityCheckInterval")) {
        m_connectivityCheckInterval = dConfig->value("ConnectivityCheckInterval").toInt() * 1000;
        emit connectivityCheckIntervalChanged(m_connectivityCheckInterval);
    } else if (key == QString("wirelessScanInterval")) {
        m_wirelessScanInterval = dConfig->value(key, 10).toInt() * 1000;
        emit wirelessScanIntervalChanged(m_wirelessScanInterval);
    } else if (key == QString("wpaEapAuthen")) {
        m_wpaEapAuthen = dConfig->value(key).toString();
        emit wpaEapAuthenChanged(m_wpaEapAuthen);
    } else if (key == QString("wpaEapAuthmethod")) {
        m_wpaEapAuthmethod = dConfig->value(key).toString();
        emit wpaEapAuthmethodChanged(m_wpaEapAuthmethod);
    } else if (key == "networkAirplaneMode") {
        m_networkAirplaneMode = dConfig->value("networkAirplaneMode").toBool();
        emit enableAirplaneModeChanged(m_networkAirplaneMode);
    } else if (key == "supportCertifiedEscape") {
        m_supportCertifiedEscape = dConfig->value("supportCertifiedEscape").toBool();
        emit supportCertifiedEscapeChanged(m_supportCertifiedEscape);
    } else if (key == "showUnauthorizedSwitch") {
        m_showUnAuthorizeSwitch = dConfig->value("showUnauthorizedSwitch").toBool();
        emit showUnAuthorizeSwitchChanged(m_showUnAuthorizeSwitch);
    } else if (key == "enableEapInput") {
        m_enableEapInput = dConfig->value("enableEapInput").toBool();
    } else if (key == "showBrowserLink") {
        m_showBrowserLink = dConfig->value("showBrowserLink").toBool();
        emit showBrowserLinkChanged(m_showBrowserLink);
    } else if (key == QString("browserUrl")) {
        m_browserUrl = dConfig->value(key).toString();
        emit browserUrlChanged(m_browserUrl);
    }
}

ConfigSetting *ConfigSetting::instance()
{
    static ConfigSetting config;
    return &config;
}

void ConfigSetting::alawaysLoadFromNM()
{
    m_alwaysFromNM = true;
}

bool ConfigSetting::serviceFromNetworkManager() const
{
    // 如果外部设置强制从NM中读取数据，则让其始终返回true
    if (m_alwaysFromNM)
        return true;

    // 其他的情况，根据配置来决定是使用NM的接口还是使用后端的接口
    return m_loadServiceFromNM;
}

QStringList ConfigSetting::networkCheckerUrls() const
{
    return m_networkUrls;
}

bool ConfigSetting::enableConnectivity() const
{
    return m_enableConnectivity;
}

bool ConfigSetting::checkPortal() const
{
    return m_checkPortal;
}

bool ConfigSetting::supportCertifiedEscape() const
{
    return m_supportCertifiedEscape;
}

bool ConfigSetting::showUnAuthorizeSwitch() const
{
    return m_showUnAuthorizeSwitch;
}

int ConfigSetting::connectivityCheckInterval() const
{
    return m_connectivityCheckInterval;
}

int ConfigSetting::wirelessScanInterval() const
{
    return m_wirelessScanInterval;
}

QString ConfigSetting::wpaEapAuthen() const
{
    return m_wpaEapAuthen;
}

QString ConfigSetting::wpaEapAuthmethod() const
{
    return m_wpaEapAuthmethod;
}

bool ConfigSetting::networkAirplaneMode() const
{
    return m_networkAirplaneMode;
}

bool ConfigSetting::enableAccountNetwork() const
{
    return m_enableAccountNetwork;
}

bool ConfigSetting::enableEapInput() const
{
    return m_enableEapInput;
}

bool ConfigSetting::dontSetIpIfConflict() const
{
    return m_dontSetIpIfConflict;
}

bool ConfigSetting::showBrowserLink() const
{
    return m_showBrowserLink;
}

QString ConfigSetting::browserUrl() const
{
    return m_browserUrl;
}
