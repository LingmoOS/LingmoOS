/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -c DBusLauncherFrame -p dbuslauncherframe org.lingmo.ocean.Launcher1.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "dbuslauncherframe.h"

/*
 * Implementation of interface class DBusLauncherFrame
 */

DBusLauncherFrame::DBusLauncherFrame(QObject *parent)
    : QDBusAbstractInterface(INTERFACE_NAME, SERVICE_PATH, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties", "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage &)));
}

DBusLauncherFrame::~DBusLauncherFrame()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties", "PropertiesChanged",  "sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage &)));
}

