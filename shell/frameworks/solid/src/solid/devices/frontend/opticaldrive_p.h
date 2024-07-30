/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_OPTICALDRIVE_P_H
#define SOLID_OPTICALDRIVE_P_H

#include "storagedrive_p.h"

namespace Solid
{
class OpticalDrivePrivate : public StorageDrivePrivate
{
public:
    OpticalDrivePrivate()
        : StorageDrivePrivate()
    {
    }
};
}

#endif // SOLID_OPTICALDRIVE_P_H
