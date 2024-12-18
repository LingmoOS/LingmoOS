/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -c DBusFileInfo -p dbusfileinfo com.lingmo.filemanager.Backend.FileInfo.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "dbusfileinfo.h"

/*
 * Implementation of interface class DBusFileInfo
 */

DBusFileInfo::DBusFileInfo(QObject *parent)
    : QDBusAbstractInterface("com.lingmo.filemanager.Backend.FileInfo", "/com/lingmo/filemanager/Backend/FileInfo", staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusFileInfo::~DBusFileInfo()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}

