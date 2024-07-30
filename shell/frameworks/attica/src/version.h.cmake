/*
 * SPDX-FileCopyrightText: 2007 Sebastian Trueg <trueg@kde.org>
 * SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _ATTICA_VERSION_H_
#define _ATTICA_VERSION_H_

#include "attica_export.h"

/// @brief Attica version as string at compile time.
#define LIBATTICA_VERSION_STRING "${ATTICA_VERSION}"

/// @brief The major Attica version number at compile time
#define LIBATTICA_VERSION_MAJOR ${ATTICA_VERSION_MAJOR}

/// @brief The minor Attica version number at compile time
#define LIBATTICA_VERSION_MINOR ${ATTICA_VERSION_MINOR}

/// @brief The Attica release version number at compile time
#define LIBATTICA_VERSION_RELEASE ${ATTICA_VERSION_PATCH}

/**
 * \brief Create a unique number from the major, minor and release number of a %Attica version
 *
 * This function can be used for preprocessing. For version information at runtime
 * use the version methods in the Attica namespace.
 */
#define LIBATTICA_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

/**
 * \brief %Attica Version as a unique number at compile time
 *
 * This macro calculates the %Attica version into a number. It is mainly used
 * through LIBATTICA_IS_VERSION in preprocessing. For version information at runtime
 * use the version methods in the Attica namespace.
 */
#define LIBATTICA_VERSION \
    LIBATTICA_MAKE_VERSION(LIBATTICA_VERSION_MAJOR,LIBATTICA_VERSION_MINOR,LIBATTICA_VERSION_RELEASE)

/**
 * \brief Check if the %Attica version matches a certain version or is higher
 *
 * This macro is typically used to compile conditionally a part of code:
 * \code
 * #if LIBATTICA_IS_VERSION(2,1)
 * // Code for Attica 2.1
 * #else
 * // Code for Attica 2.0
 * #endif
 * \endcode
 *
 * For version information at runtime
 * use the version methods in the Attica namespace.
 */
#define LIBATTICA_IS_VERSION(a,b,c) ( LIBATTICA_VERSION >= LIBATTICA_MAKE_VERSION(a,b,c) )


namespace Attica {
    /**
     * @brief Returns the major number of Attica's version, e.g.
     * 1 for %Attica 1.0.2.
     * @return the major version number at runtime.
     */
    ATTICA_EXPORT unsigned int versionMajor();

    /**
     * @brief Returns the minor number of Attica's version, e.g.
     * 0 for %Attica 1.0.2.
     * @return the minor version number at runtime.
     */
    ATTICA_EXPORT unsigned int versionMinor();

    /**
     * @brief Returns the release of Attica's version, e.g.
     * 2 for %Attica 1.0.2.
     * @return the release number at runtime.
     */
    ATTICA_EXPORT unsigned int versionRelease();

    /**
     * @brief Returns the %Attica version as string, e.g. "1.0.2".
     *
     * On contrary to the macro LIBATTICA_VERSION_STRING this function returns
     * the version number of Attica at runtime.
     * @return the %Attica version. You can keep the string forever
     */
    ATTICA_EXPORT const char* versionString();
}

#endif
