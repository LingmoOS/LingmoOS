/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KJOBWIDGETS_H
#define KJOBWIDGETS_H

#include <kwidgetsaddons_export.h>

class QObject;
class QWidget;
class QWindow;

/**
 * KJobWidgets namespace
 */
namespace KJobWidgets
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
KWIDGETSADDONS_EXPORT void setWindow(QObject *job, QWidget *widget);

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 5)
/**
 * Associate this job with a window given by @p window.
 *
 * Note that setWindow() will set the window handle too.
 *
 * @param job should be a KJob subclass
 *
 * @since 6.4
 * @deprecated since 6.5, use KJobWindows::setWindow() instead.
 */
KWIDGETSADDONS_EXPORT KWIDGETSADDONS_DEPRECATED_VERSION(6, 5, "Use KJobWindows::setWindow() instead") void setWindowHandle(QObject *job, QWindow *window);
#endif

/**
 * Return the window associated with this job.
 *
 * @param job should be a KJob subclass
 *
 * @since 6.0
 */
KWIDGETSADDONS_EXPORT QWidget *window(QObject *job);

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 5)
/**
 * Returns the window handle associated with this job.
 *
 * @param job should be a KJob subclass
 *
 * @since 6.4
 * @deprecated since 6.5, use KJobWindows::window() instead.
 */
KWIDGETSADDONS_EXPORT KWIDGETSADDONS_DEPRECATED_VERSION(6, 5, "Use KJobWindows::window() instead") QWindow *windowHandle(QObject *job);
#endif

/**
 * Updates the last user action timestamp to the given time.
 *
 * @param job should be a KJob subclass
 *
 * @since 6.0
 */
KWIDGETSADDONS_EXPORT void updateUserTimestamp(QObject *job, unsigned long time);
/**
 * Returns the last user action timestamp
 *
 * @param job should be a KJob subclass
 *
 * @since 6.0
 */
KWIDGETSADDONS_EXPORT unsigned long userTimestamp(QObject *job);
}

#endif
