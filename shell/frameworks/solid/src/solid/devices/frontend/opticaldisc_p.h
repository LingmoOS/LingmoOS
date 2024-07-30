/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_OPTICALDISC_P_H
#define SOLID_OPTICALDISC_P_H

#include "storagevolume_p.h"

namespace Solid
{
class OpticalDiscPrivate : public StorageVolumePrivate
{
public:
    OpticalDiscPrivate()
        : StorageVolumePrivate()
    {
    }
};
}

#endif
