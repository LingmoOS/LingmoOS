/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "screen.h"


Screen::Screen(QObject *parent):
    QObject(parent)
{
    m_pColorGsettings = new QGSettings(LINGMO_SETTINGS_DAEMON_COLOR_SCHEMA);
    m_outputModeEnum = QMetaEnum::fromType<eScreenMode>();

    qRegisterMetaType<OutputGammaInfo>("OutputGammaInfo");
    qRegisterMetaType<OutputGammaInfoList>("OutputGammaInfoList");
    qDBusRegisterMetaType<OutputGammaInfo>();
    qDBusRegisterMetaType<OutputGammaInfoList>();

    m_brightControlByUPM = brightControlByUPM();
    if (Screen::eBrightnessControl::powerManager == m_brightControlByUPM) {
        m_pscreen = new ScreenBrightNode(this);
    } else if (Screen::eBrightnessControl::gamma == m_brightControlByUPM) {
        m_pscreen = new ScreenGamma(this);
    }
}

Screen::~Screen()
{
    delete m_pColorGsettings;
    m_pColorGsettings = nullptr;
}

bool Screen::setScreenBrightness(QString name, uint screenBrightness)
{
    if (Screen::eBrightnessControl::unknown == m_brightControlByUPM) {
        return -1;
    }
    return m_pscreen->setScreenBrightness(name, screenBrightness);
}

uint Screen::getScreenBrightness(QString name)
{
    if (Screen::eBrightnessControl::unknown == m_brightControlByUPM) {
        return -1;
    }
    return m_pscreen->getScreenBrightness(name);
}

bool Screen::setScreensBrightness(OutputGammaInfoList outputsInfo)
{
    if (Screen::eBrightnessControl::unknown == m_brightControlByUPM) {
        return -1;
    }
    return m_pscreen->setScreensBrightness(outputsInfo);
}

bool Screen::setAllScreenSameBrightness(uint brighenss)
{
    if (Screen::eBrightnessControl::unknown == m_brightControlByUPM) {
        return -1;
    }
    return m_pscreen->setAllScreenSameBrightness(brighenss);
}

OutputGammaInfoList Screen::getAllScreenBrightness()
{
    if (Screen::eBrightnessControl::unknown == m_brightControlByUPM) {
        return OutputGammaInfoList();
    }
    return m_pscreen->getAllScreenBrightness();
}

/**
 * TODO 调用统一接口查询，设置屏幕亮度
 */
bool Screen::setPrimaryScreenBrightness(uint screenBrightness)
{
    QDBusReply<bool> reply = callBrighenss("setPrimaryBrightness", screenBrightness);
    if (false == reply.isValid()) {
        return false;
    }

    return true;
}

uint Screen::getPrimaryScreenBrightness()
{
    QDBusReply<uint> reply = callBrighenss("getPrimaryBrightness");
    if (false == reply.isValid()) {
        return -1;
    }


    return reply.value();
}

bool Screen::increaseBuiltinScreenBrightness()
{
    QDBusReply<void> reply = callMediaKeys("externalDoAction", 3,MY_NAME);
    if (false == reply.isValid()) {
        return false;
    }

    return true;
}

bool Screen::decreaseBuiltinBrightness()
{
    QDBusReply<void> reply = callMediaKeys("externalDoAction", 2,MY_NAME);
    if (false == reply.isValid()) {
        return false;
    }

    return true;
}

QString Screen::getScreenMode()
{
    QDBusReply<int> reply = callXrandr("getScreenMode",MY_NAME);
    if (false == reply.isValid()) {
        return "";
    }

    return m_outputModeEnum.valueToKey(reply.value());
}

bool Screen::setScreenMode(const QString &mode)
{
    QDBusReply<int> reply = callXrandr("setScreenMode", mode, MY_NAME);
    if (false == reply.isValid()) {
        return false;
    }

    return true;
}

bool Screen::setScreensParam(const QString &screenParam)
{
    QDBusReply<int> reply = callXrandr("setScreensParam", MY_NAME, screenParam);
    if (false == reply.isValid()) {
        return false;
    }

    return true;
}

QString Screen::getScreensParam()
{
    QDBusReply<QString> reply = callXrandr("getScreensParam", MY_NAME);
    if (false == reply.isValid()) {
        return "";
    }

    return reply.value();
}

void Screen::openDisplaySetting()
{
    QProcess::startDetached("lingmo-control-center -m display");
}

bool Screen::setColorTemperature(QString appName, int colorTemp)
{
    return true;
}

bool Screen::setEyeCareEnabled(bool state)
{
   QString ret = m_pColorGsettings->set(LINGMO_SETTINGS_DAEMON_COLOR_EYS_CARE, state);
   return ret.isEmpty();
}

bool Screen::isEyeCareEnabled()
{
   return m_pColorGsettings->get(LINGMO_SETTINGS_DAEMON_COLOR_EYS_CARE).toBool();
}

bool Screen::setNightModeEnable(bool state)
{
    QString ret = m_pColorGsettings->set(LINGMO_SETTINGS_DAEMON_COLOR_NIGHT_MODE, state);
    return ret.isEmpty();
}

bool Screen::isNightModeEnable()
{
    return  m_pColorGsettings->get(LINGMO_SETTINGS_DAEMON_COLOR_NIGHT_MODE).toBool();
}

Screen::eBrightnessControl Screen::brightControlByUPM()
{
    QDBusInterface powerInterface(LINGMO_POWER_MANAGER_DBUS_NAME,
                                  LINGMO_POWER_MANAGER_DBUS_PATH,
                                 LINGMO_POWER_MANAGER_DBUS_INTERFACE,
                                  QDBusConnection::systemBus());

    QDBusReply<bool> reply = powerInterface.call("CanSetBrightness");

    if (reply.isValid()) {
        if (reply.value()) {
            return Screen::eBrightnessControl::powerManager;
        } else {
            return Screen::eBrightnessControl::gamma;
        }
    }
    qDebug() << "powermanager's systembus call error!"<<reply.error();
    return Screen::eBrightnessControl::unknown;
}

void Screen::watchUpmRegisterDbusServer()
{
    qDebug() << "ready connect dbus server!";
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    auto watcher = new QDBusServiceWatcher(QStringLiteral(LINGMO_POWER_MANAGER_DBUS_NAME), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &MediaKeyBlockShortcutPrivate::serviceOwnerChanged);
#else
    connect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &Screen::serviceOwnerChanged);
#endif
}

QDBusMessage Screen::callGamma(const QString &method, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4)
{
    QDBusInterface globalManagerInterface("org.lingmo.SettingsDaemon",
                                  "/org/lingmo/SettingsDaemon/GammaManager",
                                  "org.lingmo.SettingsDaemon.GammaManager");

    return globalManagerInterface.call(method,arg1,arg2,arg3,arg4);
}

QDBusMessage Screen::callXrandr(const QString &method, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4)
{
    QDBusInterface globalManagerInterface("org.lingmo.SettingsDaemon",
                                  "/org/lingmo/SettingsDaemon/xrandr",
                                  "org.lingmo.SettingsDaemon.xrandr");

    return globalManagerInterface.call(method,arg1,arg2,arg3,arg4);;
}

QDBusMessage Screen::callBrighenss(const QString &method, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4)
{
    QDBusInterface globalManagerInterface("org.lingmo.SettingsDaemon",
                                  "/GlobalBrightness",
                                  "org.lingmo.SettingsDaemon.Brightness");


    return globalManagerInterface.call(method,arg1,arg2,arg3,arg4);
}

QDBusMessage Screen::callMediaKeys(const QString &method, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4)
{
    QDBusInterface mediaKeysInterface("org.lingmo.SettingsDaemon",
                                  "/org/lingmo/SettingsDaemon/MediaKeys",
                                  "org.lingmo.SettingsDaemon.MediaKeys");


    return mediaKeysInterface.call(method,arg1,arg2,arg3,arg4);
}

void Screen::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    if (name.isEmpty()) {
        return;
    }

    if (!name.startsWith(LINGMO_POWER_MANAGER_DBUS_NAME)) {
        //过滤不符合mpris的seivice
        return;
    }

    if (nullptr != m_pscreen) {
        delete m_pscreen;
        m_pscreen = nullptr;
    }

    m_brightControlByUPM = brightControlByUPM();
    if (Screen::eBrightnessControl::powerManager == m_brightControlByUPM) {
        m_pscreen = new ScreenBrightNode(this);
    } else if (Screen::eBrightnessControl::gamma == m_brightControlByUPM) {
        m_pscreen = new ScreenGamma(this);
    }

    disconnect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &Screen::serviceOwnerChanged);
}
