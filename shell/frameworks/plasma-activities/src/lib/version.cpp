/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "version.h"

namespace KActivities
{
unsigned int version()
{
    return PLASMA_ACTIVITIES_VERSION;
}

unsigned int versionMajor()
{
    return PLASMA_ACTIVITIES_VERSION_MAJOR;
}

unsigned int versionMinor()
{
    return PLASMA_ACTIVITIES_VERSION_MINOR;
}

unsigned int versionRelease()
{
    return PLASMA_ACTIVITIES_VERSION_RELEASE;
}

const char *versionString()
{
    return PLASMA_ACTIVITIES_VERSION_STRING;
}

} // KActivities namespace
