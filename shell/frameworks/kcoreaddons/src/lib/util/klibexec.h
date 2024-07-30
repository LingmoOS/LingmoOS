/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#ifndef KLIBEXEC_H
#define KLIBEXEC_H

#include <kcoreaddons_export.h>

#include <QString>
#include <QStringList>

/**
 * @brief Utility functions around libexec.
 */
namespace KLibexec
{

#ifndef K_DOXYGEN
// Internal helpers. Do not use these but the inline variants.
KCOREADDONS_EXPORT QString pathFromAddress(const QString &relativePath, void *address);
KCOREADDONS_EXPORT QStringList pathCandidates(const QString &relativePath);
#endif

/**
 * @brief Absolute libexec path resolved in relative relation to the current shared object.
 *
 * This function helps locate the absolute libexec path relative to the caller's binary artifact.
 *
 * For example:
 *
 * - Your source gets built with prefix /usr
 * - Your binary artifact's presumed absolute path will be `/usr/lib/libfoobar.so`
 * - You call `KLibexec::path("libexec/foobar")`
 *
 * Scenario 1 - The binaries are actually installed in /usr:
 * - The function's output is `/usr/lib/libexec/foobar/` (resolved relatively from `/usr/lib/libfoobar.so`)
 *
 * Scenario 2 - The **same** binaries are installed in /opt (or moved there):
 * - The function's output is `/opt/lib/libexec/foobar/` (resolved relatively from `/opt/lib/libfoobar.so`)
 *
 * @param relativePath relative element to append (e.g. "libexec/foobar" resulting in /usr/lib/libexec/foobar/ as output)
 *   when called with an empty string you effectively get the directory of your binary artifact.
 * @return QString absolute libexec path or empty string if it cannot be resolved
 * @since 5.91
 */
inline QString path(const QString &relativePath)
{
    // this MUST be inline so that the marker address is in the calling object!
    static int marker = 0;
    return pathFromAddress(relativePath, &marker);
}

/**
 * @brief default paths list for KDE Frameworks
 *
 * This function returns a fairly opinionated list of paths you can feed into QStandardPaths. The list includes
 * various standard locations for Qt and KDE Frameworks and should generally be sensible for most use cases.
 * You may wish to append the absolute installation path as final fallback.
 *
 * @warning The precise content and order of the list is an implementation detail and not expected to remain stable!
 *
 * @param relativePath see path() - not all paths get this appended!
 * @return QStringList list of search paths
 * @since 5.91
 */
inline QStringList kdeFrameworksPaths(const QString &relativePath)
{
    // intentionally inline because path must be inline
    return pathCandidates(path(relativePath));
}

} // namespace KLibexec

#endif // KLIBEXEC_H
