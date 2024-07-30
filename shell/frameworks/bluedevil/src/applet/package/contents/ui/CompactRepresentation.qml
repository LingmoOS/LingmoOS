/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

MouseArea {
    id: root

    required property PlasmoidItem plasmoidItem
    required property PlasmaCore.Action toggleBluetoothAction

    readonly property bool inPanel: [
        PlasmaCore.Types.TopEdge,
        PlasmaCore.Types.RightEdge,
        PlasmaCore.Types.BottomEdge,
        PlasmaCore.Types.LeftEdge,
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

    Kirigami.Icon {
        anchors.fill: parent
        source: Plasmoid.icon
        active: root.containsMouse
    }
}
