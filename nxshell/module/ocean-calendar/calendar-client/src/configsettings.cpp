// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "configsettings.h"

#include <DStandardPaths>
#include <QDebug>

DCORE_USE_NAMESPACE;

CConfigSettings::CConfigSettings()
{
    init();
}

CConfigSettings::~CConfigSettings()
{
    releaseInstance();
}

void CConfigSettings::init()
{
    //如果为空则创建
    if (m_settings == nullptr) {
        auto configFilepath = DStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).value(0) + "/config.ini";
        m_settings = new QSettings(configFilepath, QSettings::IniFormat);
    }
    initSetting();
}

void CConfigSettings::initSetting()
{
    m_userSidebarStatus = value("userSidebarStatus", true).toBool();
}

/**
 * @brief CConfigSettings::releaseInstance  释放配置指针
 */
void CConfigSettings::releaseInstance()
{
    delete m_settings;
    m_settings = nullptr;
}

CConfigSettings *CConfigSettings::getInstance()
{
    static CConfigSettings configSettings;
    return &configSettings;
}

void CConfigSettings::sync()
{
    m_settings->sync();
}

QVariant CConfigSettings::value(const QString &key, const QVariant &defaultValue)
{
    return m_settings->value(key, defaultValue);
}

//设置对应key的值
void CConfigSettings::setOption(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
}

//移除对应的配置信息
void CConfigSettings::remove(const QString &key)
{
    m_settings->remove(key);
}

//判断是否包含对应的key
bool CConfigSettings::contains(const QString &key) const
{
    return m_settings->contains(key);
}

CConfigSettings *CConfigSettings::operator->() const
{
    return getInstance();
}

bool CConfigSettings::getUserSidebarStatus()
{
    return m_userSidebarStatus;
}

void CConfigSettings::setUserSidebarStatus(bool status)
{
    m_userSidebarStatus = status;
    setOption("userSidebarStatus", m_userSidebarStatus);
}
