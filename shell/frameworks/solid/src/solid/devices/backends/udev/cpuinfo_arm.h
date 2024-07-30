/*
    SPDX-FileCopyrightText: 2021 Alexey Minnekhanov <alexeymin@postmarketos.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_CPUINFO_ARM_H
#define SOLID_BACKENDS_UDEV_CPUINFO_ARM_H

/**
 * Detect if we are compiling for ARM 32-bit or 64-bit platforms.
 * This works at least for gcc and clang (they both have the same
 * default list of defined macros, which can be obtained by:
 * - for clang: clang -dM -E - < /dev/null
 * - for gcc:  gcc -march=native -dM -E - </dev/null
 */

#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH) || defined(__ARM_EABI__)
#define BUILDING_FOR_ARM_TARGET
#endif

// don't even build the following code for non-ARM platforms
#ifdef BUILDING_FOR_ARM_TARGET

#include <QString>

namespace Solid
{
namespace Backends
{
namespace UDev
{
/**
 * @brief The ArmIdPart struct
 * Describes specfic CPU Model. Is used to
 * convert numerical CPU ID to name string.
 */
struct ArmIdPart {
    const int id; //! CPU model ID; -1 means end of list
    const char *name; //! CPU human-readable name
};

struct ArmCpuImplementer {
    const int id; //! CPU vendor ID
    const struct ArmIdPart *parts; //! pointer to an array of parts, last elemnt will have ID -1
    const char *name; //! CPU vendor name
};

const ArmCpuImplementer *findArmCpuImplementer(int vendorId);
QString findArmCpuModel(int vendorId, int modelId);

} // namespace UDev
} // namespace Backends
} // namespace Solid

#endif // BUILDING_FOR_ARM_TARGET

#endif // SOLID_BACKENDS_UDEV_CPUINFO_ARM_H
