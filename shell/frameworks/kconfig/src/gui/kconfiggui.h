/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Ettrich <ettrich@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KCONFIGGUI_H
#define KCONFIGGUI_H

#include <kconfiggui_export.h>

#include <QString>

class KConfig;

/**
 * Interface-related functions.
 */
namespace KConfigGui
{
/**
 * Returns the current application session config object.
 *
 * @note If Qt is built without session manager support, i.e.
 * QT_NO_SESSIONMANAGER is defined, this by default will return
 * nullptr, unless a custom config  has been set via
 * @c setSessionConfig.
 *
 * @return A pointer to the application's instance specific
 * KConfig object.
 * @see KConfig
 */
KCONFIGGUI_EXPORT KConfig *sessionConfig();

/**
 * Replaces the current application session config object.
 *
 * @param id  new session id
 * @param key new session key
 *
 * @since 5.11
 */
KCONFIGGUI_EXPORT void setSessionConfig(const QString &id, const QString &key);

/**
 * Indicates if a session config has been created for that application
 * (i.e.\ if sessionConfig() got called at least once)
 *
 * @return @c true if a sessionConfig object was created, @c false otherwise
 */
KCONFIGGUI_EXPORT bool hasSessionConfig();
}

#endif // KCONFIGGUI_H
