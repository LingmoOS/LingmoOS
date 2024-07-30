/*
    SPDX-FileCopyrightText: 2008-2016 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_ACTIVITIES_VERSION_BIN_H
#define PLASMA_ACTIVITIES_VERSION_BIN_H

/** @file version.h <PlasmaActivities/Version> */

#include "plasma_activities_export.h"
#include <plasma_activities_version.h>

#define PLASMA_ACTIVITIES_VERSION_RELEASE PLASMA_ACTIVITIES_VERSION_PATCH

/**
 * Namespace for everything in libkactivities
 */
namespace KActivities
{
/**
 * The runtime version of libkactivities
 */
PLASMA_ACTIVITIES_EXPORT unsigned int version();

/**
 * The runtime major version of libkactivities
 */
PLASMA_ACTIVITIES_EXPORT unsigned int versionMajor();

/**
 * The runtime major version of libkactivities
 */
PLASMA_ACTIVITIES_EXPORT unsigned int versionMinor();

/**
 * The runtime major version of libkactivities
 */
PLASMA_ACTIVITIES_EXPORT unsigned int versionRelease();

/**
 * The runtime version string of libkactivities
 */
PLASMA_ACTIVITIES_EXPORT const char *versionString();

} // KActivities namespace

#endif // PLASMA_ACTIVITIES_VERSION_BIN_H
