/*
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "storageaccess.h"

Solid::Ifaces::StorageAccess::~StorageAccess()
{
}

bool Solid::Ifaces::StorageAccess::canCheck() const
{
    return false;
}

bool Solid::Ifaces::StorageAccess::check()
{
    return false;
}

bool Solid::Ifaces::StorageAccess::canRepair() const
{
    return false;
}

bool Solid::Ifaces::StorageAccess::repair()
{
    return false;
}

void Solid::Ifaces::StorageAccess::repairRequested(const QString &udi)
{
    Q_UNUSED(udi);
}

void Solid::Ifaces::StorageAccess::repairDone(Solid::ErrorType error, QVariant resultData, const QString &udi)
{
    Q_UNUSED(error);
    Q_UNUSED(resultData);
    Q_UNUSED(udi);
}
