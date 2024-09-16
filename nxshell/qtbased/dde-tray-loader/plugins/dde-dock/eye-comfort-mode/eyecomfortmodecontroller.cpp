// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "eyecomfortmodecontroller.h"

#include <QVariant>
#include <QDebug>

#define COLOR_TEMPERATURE_ENABLED "ColorTemperatureEnabled"
#define SUPPORT_COLOR_TEMPERATURE "SupportColorTemperature"

EyeComfortModeController::EyeComfortModeController()
        : m_displayInter(new QDBusInterface("com.deepin.daemon.Display", "/com/deepin/daemon/Display", "com.deepin.daemon.Display", QDBusConnection::sessionBus(), this))
        , m_themeInter(new DBusTheme("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", QDBusConnection::sessionBus(), this))
        , m_eyeComfortModeEnabled(false)
        , m_supportColorTemperature(false)
{
    if (!m_displayInter) {
        return;
    }

    QDBusConnection::sessionBus().connect("com.deepin.daemon.Display", "/com/deepin/daemon/Display", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
    connect(m_themeInter, &DBusTheme::GlobalThemeChanged, this, &EyeComfortModeController::globalThemeChanged);

    m_eyeComfortModeEnabled = m_displayInter->property(COLOR_TEMPERATURE_ENABLED).toBool();
    m_supportColorTemperature = m_displayInter->property(SUPPORT_COLOR_TEMPERATURE).toBool();
}

void EyeComfortModeController::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    if (changedProperties.contains(COLOR_TEMPERATURE_ENABLED)) {
        m_eyeComfortModeEnabled = changedProperties.value(COLOR_TEMPERATURE_ENABLED, false).toBool();
        Q_EMIT eyeComfortModeChanged(m_eyeComfortModeEnabled);
    }

    if (changedProperties.contains(SUPPORT_COLOR_TEMPERATURE)) {
        m_supportColorTemperature = changedProperties.value(SUPPORT_COLOR_TEMPERATURE, false).toBool();
        Q_EMIT supportColorTemperatureChanged(m_supportColorTemperature);
    }
}

void EyeComfortModeController::setGtkTheme(const QString &value)
{
    if (m_themeInter->gtkTheme() == value)
        return;

    m_themeInter->Set("gtk", value);
}

QString EyeComfortModeController::gtkTheme() const
{
    return m_themeInter->gtkTheme();
}

QString EyeComfortModeController::globalTheme() const
{
    return m_themeInter->globalTheme();
}

void EyeComfortModeController::enable(bool enable)
{
    m_displayInter->setProperty(COLOR_TEMPERATURE_ENABLED, !isEyeComfortModeEnabled());
}

void EyeComfortModeController::setGlobalTheme(const QString &value)
{
    if (m_themeInter->globalTheme() == value)
        return;

    m_themeInter->Set("globaltheme", value);
}
