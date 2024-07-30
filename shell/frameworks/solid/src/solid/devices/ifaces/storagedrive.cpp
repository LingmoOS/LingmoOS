/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "storagedrive.h"

Solid::Ifaces::StorageDrive::~StorageDrive()
{
}

QDateTime Solid::Ifaces::StorageDrive::timeDetected() const
{
    return QDateTime();
}

QDateTime Solid::Ifaces::StorageDrive::timeMediaDetected() const
{
    return QDateTime();
}
