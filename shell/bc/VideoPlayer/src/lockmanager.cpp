/*
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "lockmanager.h"
#include "screensaverdbusinterface.h"

#include <QDBusConnection>

LockManager::LockManager(QObject *parent)
    : QObject(parent)
    , m_inhibit()
{
    m_iface = new OrgFreedesktopScreenSaverInterface(
                QStringLiteral("org.freedesktop.ScreenSaver"),
                QStringLiteral("/org/freedesktop/ScreenSaver"),
                QDBusConnection::sessionBus(),
                this);
}

void LockManager::setInhibitionOff()
{
    m_iface->UnInhibit(m_cookie);
}

void LockManager::setInhibitionOn()
{
    m_cookie = m_iface->Inhibit(
                QStringLiteral("Lingmo Video Player"),
                "Playing video.");
}
