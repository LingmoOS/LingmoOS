/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import org.kde.lingmo.core as LingmoCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.kquickcontrolsaddons 2.0
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.private.nanoshell 2.0 as NanoShell
import org.kde.lingmoui 2.20 as LingmoUI

Item {
    id: root
    objectName: "org.kde.desktop-CompactApplet"
    anchors.fill: parent

    property Item fullRepresentation
    property Item compactRepresentation
    property Item expandedFeedback: expandedItem
    property PlasmoidItem plasmoidItem

    property Item rootItem: {
        var item = root
        while (item.parent) {
            item = item.parent;
        }
        return item;
    }
    onCompactRepresentationChanged: {
        if (compactRepresentation) {
            compactRepresentation.parent = compactRepresentationParent;
            compactRepresentation.anchors.fill = compactRepresentationParent;
            compactRepresentation.visible = true;
        }
        root.visible = true;
    }

    onFullRepresentationChanged: {

        if (!fullRepresentation) {
            return;
        }

        fullRepresentation.parent = appletParent;
        fullRepresentation.anchors.fill = fullRepresentation.parent;
        fullRepresentation.anchors.margins = appletParent.margins.top;
    }

    FocusScope {
        id: compactRepresentationParent
        anchors.fill: parent
        activeFocusOnTab: true
        onActiveFocusChanged: {
            // When the scope gets the active focus, try to focus its first descendant,
            // if there is on which has activeFocusOnTab
            if (!activeFocus) {
                return;
            }
            let nextItem = nextItemInFocusChain();
            let candidate = nextItem;
            while (candidate.parent) {
                if (candidate === compactRepresentationParent) {
                    nextItem.forceActiveFocus();
                    return;
                }
                candidate = candidate.parent;
            }
        }
        // This object name is needed for GUI testing. all gui tests in lingmo-workspace are done with lingmo-nano
        objectName: "expandApplet"
        Accessible.name: root.plasmoidItem?.toolTipMainText??""
        Accessible.description: i18nd("lingmo_shell_org.kde.lingmo.nano", "Open %1", root.plasmoidItem?.toolTipSubText??"")
        Accessible.role: Accessible.Button
        Accessible.onPressAction: Plasmoid.activated()
    }

    Rectangle {
        id: expandedItem
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.top
        }

        height: LingmoUI.Units.smallSpacing
        color: LingmoUI.Theme.highlightColor
        visible: plasmoid.formFactor != LingmoCore.Types.Planar && Boolean(plasmoidItem?.expanded)
    }

    Connections {
        target: plasmoidItem
        function onExpandedChanged() {
            if (plasmoidItem.expanded) {
                expandedOverlay.showFullScreen()
            } else {
                expandedOverlay.visible = false;
            }
        }
    }

    NanoShell.FullScreenOverlay {
        id: expandedOverlay
        color: Qt.rgba(0, 0, 0, 0.6)
        visible: plasmoidItem?.expanded??false
        width: Screen.width
        height: Screen.height
        MouseArea {
            anchors.fill: parent
            onClicked: plasmoidItem.expanded = false
        }

        KSvg.FrameSvgItem {
            id: appletParent
            imagePath: "widgets/background"
            //used only indesktop mode, not panel

            x: Math.max(0, Math.min(parent.width - width - LingmoUI.Units.gridUnit, Math.max(LingmoUI.Units.gridUnit, root.mapToItem(root.rootItem, 0, 0).x + root.width / 2 - width / 2)))
            y: Math.max(0, Math.min(parent.height - height - LingmoUI.Units.gridUnit, Math.max(LingmoUI.Units.gridUnit, root.mapToItem(root.rootItem, 0, 0).y + root.height / 2 - height / 2)))
            width: Math.min(expandedOverlay.width,  Math.max(Math.max(root.fullRepresentation?.implicitWidth ?? 0, LingmoUI.Units.gridUnit * 15), plasmoidItem?.switchWidth ?? 0) * 1.5)
            height: Math.min(expandedOverlay.height, Math.max(Math.max(root.fullRepresentation?.implicitHeight ?? 0, LingmoUI.Units.gridUnit * 15), plasmoidItem?.switchHeight ?? 0) * 1.5)
        }
    }
}
