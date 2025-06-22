/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KJOBWIDGETS_H
#define KJOBWIDGETS_H

#include <kjobwidgets_export.h>

class QWidget;
class QWindow;
class KJob;

/**
 * KJobWidgets namespace
 */
namespace KJobWidgets
{
/**
 * Associate this job with a window given by @p window.
 * This is used:
 * @li by KDialogJobUiDelegate as parent widget for error messages
 * @li by KWidgetJobTracker as parent widget for progress dialogs
 * @li by KIO::AbstractJobInteractionInterface as parent widget for rename/skip dialogs
 * and possibly more.
 * @li by KIO::DropJob as parent widget of popup menus.
 * This is required on Wayland to properly position the menu.
 *
 * @since 5.0
 */
KJOBWIDGETS_EXPORT void setWindow(KJob *job, QWidget *widget);

/**
 * Return the window associated with this job.
 * @since 5.0
 */
KJOBWIDGETS_EXPORT QWidget *window(KJob *job);

/**
 * Updates the last user action timestamp to the given time.
 * @since 5.0
 */
KJOBWIDGETS_EXPORT void updateUserTimestamp(KJob *job, unsigned long time);
/**
 * Returns the last user action timestamp
 * @since 5.0
 */
KJOBWIDGETS_EXPORT unsigned long userTimestamp(KJob *job);
}

/**
 * KJobWindows namespace
 */
namespace KJobWindows
{
/**
 * Associate this job with a window given by @p window.
 * This is used:
 * @li by KDialogJobUiDelegate as parent widget for error messages
 * @li by KWidgetJobTracker as parent widget for progress dialogs
 * @li by KIO::AbstractJobInteractionInterface as parent widget for rename/skip dialogs
 * and possibly more.
 * @li by KIO::DropJob as parent widget of popup menus.
 * This is required on Wayland to properly position the menu.
 *
 * @since 5.42
 */
KJOBWIDGETS_EXPORT void setWindow(KJob *job, QWindow *window);

/**
 * Return the window associated with this job.
 * @since 5.42
 */
KJOBWIDGETS_EXPORT QWindow *window(KJob *job);
}

#endif
