/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KMIMETYPE_EDITOR_H
#define KMIMETYPE_EDITOR_H

#include <QWidget>
#include <kwidgetsaddons_export.h>

/**
 * This namespace provides a method for starting the file types editor.
 * @since 5.4
 */
namespace KMimeTypeEditor
{
/**
 * Starts the file types editor for a given MIME type.
 *
 * Displays an error message if the mimetype editing tool (keditfiletype)
 * couldn't be launched.
 *
 * @param mimeType the MIME type to edit, e.g. "text/plain"
 * @param widget the parent widget
 * @since 5.4
 */
KWIDGETSADDONS_EXPORT void editMimeType(const QString &mimeType, QWidget *widget);
}

#endif
