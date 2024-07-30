/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid

MouseArea {
    id: root

    required property PlasmoidItem plasmoidItem
    required property LingmoCore.Action toggleBluetoothAction

    readonly property bool inPanel: [
        LingmoCore.Types.TopEdge,
        LingmoCore.Types.RightEdge,
        LingmoCore.Types.BottomEdge,
        LingmoCore.Types.LeftEdge,
    ].includes(Plasmoid.location)

    acceptedButtons: Qt.LeftButton | Qt.MiddleButton

    property bool wasExpanded

    onPressed: mouse => {
        wasExpanded = plasmoidItem.expanded
    }

    onClicked: mouse => {
        if (mouse.button === Qt.MiddleButton) {
            root.toggleBluetoothAction.trigger();
        } else {
            plasmoidItem.expanded = !wasExpanded;
        }
    }

    hoverEnabled: true

    LingmoUI.Icon {
        anchors.fill: parent
        source: Plasmoid.icon
        active: root.containsMouse
    }
}
