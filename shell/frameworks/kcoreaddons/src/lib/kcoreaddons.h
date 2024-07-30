/*
    This file is part of the KDE Libraries

    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOREADDONS_H
#define KCOREADDONS_H

#include <QString>
#include <kcoreaddons_export.h>

/**
 * @namespace KCoreAddons
 * Provides utility functions for metadata about the KCoreAddons library.
 */
namespace KCoreAddons
{
/**
 * Returns the version number of KCoreAddons at run-time as a string (for example, "5.19.0").
 * This may be a different version than the version the application was compiled against.
 * @since 5.20
 */
KCOREADDONS_EXPORT QString versionString();

/**
 * Returns a numerical version number of KCoreAddons at run-time in the form 0xMMNNPP
 * (MM = major, NN = minor, PP = patch)
 * This can be compared using the macro QT_VERSION_CHECK.
 *
 * For example:
 * \code
 * if (KCoreAddons::version() < QT_VERSION_CHECK(5,19,0))
 * \endcode
 *
 * This may be a different version than the version the application was compiled against.
 * @since 5.20
 */
KCOREADDONS_EXPORT unsigned int version();
}

#endif
