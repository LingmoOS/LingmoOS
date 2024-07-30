/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KJOBWINDOWS_H
#define KJOBWINDOWS_H

#include <kguiaddons_export.h>

class QWindow;
class QObject;

/**
 * KJobWindows namespace
 */
namespace KJobWindows
{
/**
 * Associate this job with a window given by @p window.
 *
 * @param job should be a KJob subclass
 *
 * This is used:
 * @li by KDialogJobUiDelegate as parent widget for error messages
 * @li by KWidgetJobTracker as parent widget for progress dialogs
 * @li by KIO::AbstractJobInteractionInterface as parent widget for rename/skip dialogs
 * and possibly more.
 * @li by KIO::DropJob as parent widget of popup menus.
 * This is required on Wayland to properly position the menu.
 *
 * @since 6.0
 */
KGUIADDONS_EXPORT void setWindow(QObject *job, QWindow *window);

/**
 * Return the window associated with this job.
 *
 * @param job should be a KJob subclass
 *
 * @since 6.0
 */
KGUIADDONS_EXPORT QWindow *window(QObject *job);
}

#endif
