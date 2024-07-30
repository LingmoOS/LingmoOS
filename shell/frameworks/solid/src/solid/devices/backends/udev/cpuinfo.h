/*
    SPDX-FileCopyrightText: 2010 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_CPUINFO_H
#define SOLID_BACKENDS_UDEV_CPUINFO_H

#include <QString>

namespace Solid
{
namespace Backends
{
namespace UDev
{
/**
 * Extracts vendor from /proc/cpuinfo for a given processor
 */
QString extractCpuVendor(int processorNumber);
/**
 * Extracts model from /proc/cpuinfo for a given processor
 */
QString extractCpuModel(int processorNumber);
/**
 * Extracts speed from /proc/cpuinfo for a given processor
 */
int extractCurrentCpuSpeed(int processorNumber);

}
}
}

#endif // SOLID_BACKENDS_UDEV_CPUINFO_H
