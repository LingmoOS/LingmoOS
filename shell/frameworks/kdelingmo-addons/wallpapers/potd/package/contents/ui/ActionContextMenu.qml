/*
 *   SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

QQC2.Menu {
    id: contextMenu

    /**
     * Always show all actions regardless of visible property
     */
    property bool showAllActions: false

    /**
     * A list of extra actions in the context menu.
     */
    property list<QtObject> actions

    Repeater {
        model: contextMenu.actions
        delegate: QQC2.MenuItem {
            text: modelData.text || modelData.tooltip
            icon.name: modelData.iconName
            onTriggered: modelData.trigger()
            enabled: modelData.enabled
            visible: modelData.visible || contextMenu.showAllActions

            Accessible.description: modelData.Accessible.description
        }
    }

    onClosed: if (parent) parent.forceActiveFocus()
}
