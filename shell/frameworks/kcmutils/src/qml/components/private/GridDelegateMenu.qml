/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

QQC2.Menu {
    id: menu

    function trigger() {
        parent.clicked();
        if (parent.actions.length > 0) {
            popup(parent, thumbnail.x, thumbnail.y + thumbnail.height);
        }
    }

    onClosed: parent.forceActiveFocus()

    Repeater {
        model: menu.parent.actions
        delegate: QQC2.MenuItem {
            required property Kirigami.Action modelData

            text: modelData.text || modelData.tooltip
            icon.name: modelData.icon.name
            enabled: modelData.enabled
            visible: modelData.visible

            onTriggered: modelData.trigger()
        }
    }
}
