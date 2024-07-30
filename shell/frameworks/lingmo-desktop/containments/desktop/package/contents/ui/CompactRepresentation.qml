/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.lingmoui 2.20 as LingmoUI

DragDrop.DropArea {
    readonly property bool inPanel: [
        LingmoCore.Types.TopEdge,
        LingmoCore.Types.LeftEdge,
        LingmoCore.Types.RightEdge,
        LingmoCore.Types.BottomEdge,
    ].includes(Plasmoid.location)

    Layout.minimumWidth: Plasmoid.formFactor === LingmoCore.Types.Horizontal ? height : LingmoUI.Units.iconSizes.small
    Layout.minimumHeight: Plasmoid.formFactor === LingmoCore.Types.Vertical ? width : (LingmoUI.Units.iconSizes.small + 2 * LingmoUI.Units.iconSizes.sizeForLabels)

    property Item folderView: null

    onContainsDragChanged: contained => {
        if (containsDrag) {
            hoverActivateTimer.restart();
        } else {
            hoverActivateTimer.stop();
        }
    }

    onDrop: event => {
        folderView.model.dropCwd(event);
    }

    preventStealing: true

    function toggle() {
        root.expanded = !root.expanded;
    }

    LingmoUI.Icon {
        id: icon

        anchors.fill: parent

        active: mouseArea.containsMouse

        source: Plasmoid.configuration.useCustomIcon ? Plasmoid.configuration.icon : folderView.model.iconName
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: mouse => toggle()
    }

    Timer {
        id: hoverActivateTimer

        interval: root.hoverActivateDelay

        onTriggered: toggle()
    }
}
