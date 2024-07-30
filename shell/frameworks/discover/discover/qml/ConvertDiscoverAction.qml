/*
 *   SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQml
import org.kde.lingmoui as LingmoUI
import org.kde.discover as Discover

/*
 * Converts a DiscoverAction into a LingmoUI.Action so we can use DiscoverActions
 * with QQC2 components
 */
LingmoUI.Action {
    required property Discover.DiscoverAction action

    icon.name: action.iconName
    text: action.text
    tooltip: action.toolTip
    visible: action.visible
    onTriggered: action.trigger()
}
