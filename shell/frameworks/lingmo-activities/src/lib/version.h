/*
    SPDX-FileCopyrightText: 2008-2016 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMO_ACTIVITIES_VERSION_BIN_H
#define LINGMO_ACTIVITIES_VERSION_BIN_H

/** @file version.h <LingmoActivities/Version> */

#include "lingmo_activities_export.h"
#include <lingmo_activities_version.h>

#define LINGMO_ACTIVITIES_VERSION_RELEASE LINGMO_ACTIVITIES_VERSION_PATCH

/**
 * Namespace for everything in libkactivities
 */
namespace KActivities
{
/**
 * The runtime version of libkactivities
 */
LINGMO_ACTIVITIES_EXPORT unsigned int version();

/**
 * The runtime major version of libkactivities
 */
LINGMO_ACTIVITIES_EXPORT unsigned int versionMajor();

/**
 * The runtime major version of libkactivities
 */
LINGMO_ACTIVITIES_EXPORT unsigned int versionMinor();

/**
 * The runtime major version of libkactivities
 */
LINGMO_ACTIVITIES_EXPORT unsigned int versionRelease();

/**
 * The runtime version string of libkactivities
 */
LINGMO_ACTIVITIES_EXPORT const char *versionString();

} // KActivities namespace

#endif // LINGMO_ACTIVITIES_VERSION_BIN_H
