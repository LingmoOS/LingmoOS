/*
    SPDX-FileCopyrightText: 2024 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTYLEMANAGER_H
#define KSTYLEMANAGER_H

#include <kconfigwidgets_export.h>

class QAction;
class QObject;

/**
 * Manager for Qt widget styles
 */
namespace KStyleManager
{

/**
 * Enforces the style configured by the user with fallback to the Breeze style.
 *
 * (following the settings in the application configuration with fallback to kdeglobals)
 *
 * Must be called after one has constructed the QApplication.
 *
 * If the application is already using the KDE platform theme, the style will not
 * be touched and the platform theme will ensure proper styling.
 *
 * @since 6.3
 */
KCONFIGWIDGETS_EXPORT void initStyle();

/**
 * Creates an action to configure the current used style.
 *
 * If the application is running using the KDE platform theme, this will
 * just return a disabled and hidden action, else it will provide an action to
 * show a menu to select the style to use for this applications.
 *
 * @since 6.3
 */
KCONFIGWIDGETS_EXPORT QAction *createConfigureAction(QObject *parent = nullptr);
}

#endif
