/*
    SPDX-FileCopyrightText: 2005 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ifaces/devicemanager.h"

Solid::Ifaces::DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
}

Solid::Ifaces::DeviceManager::~DeviceManager()
{
}

#include "moc_devicemanager.cpp"
