/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wingenericinterface.h"

using namespace Solid::Backends::Win;

WinGenericInterface::WinGenericInterface(WinDevice *device)
    : WinInterface(device)
{
}

QVariant WinGenericInterface::property(const QString &key) const
{
    Q_UNUSED(key)
    return QVariant();
}

QMap<QString, QVariant> Solid::Backends::Win::WinGenericInterface::allProperties() const
{
    return QMap<QString, QVariant>();
}

bool WinGenericInterface::propertyExists(const QString &key) const
{
    Q_UNUSED(key)
    return false;
}

#include "moc_wingenericinterface.cpp"
