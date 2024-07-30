/*
    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEMEMENU_H
#define KCOLORSCHEMEMENU_H

class KActionMenu;
class KColorSchemeManager;
class QObject;

#include <kconfigwidgets_export.h>

/**
 *  A menu for switching color schemes
 */
namespace KColorSchemeMenu
{
/**
 * Creates a KActionMenu populated with all the available color schemes.
 * All actions are in an action group and when one of the actions is triggered the scheme
 * referenced by this action is activated.
 *
 * @since 5.107
 */
KCONFIGWIDGETS_EXPORT KActionMenu *createMenu(KColorSchemeManager *manager, QObject *parent = nullptr);
}

#endif
