/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.lingmo.core as LingmoCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmo.plasmoid 2.0

KSvg.SvgItem {
    id: arrow

    anchors {
        bottom: arrow.parent.bottom
        horizontalCenter: iconBox.horizontalCenter
    }

    visible: parent.model.IsGroupParent

    states: [
        State {
            name: "top"
            when: Plasmoid.location === LingmoCore.Types.TopEdge
            AnchorChanges {
                target: arrow
                anchors.top: arrow.parent.top
                anchors.left: undefined
                anchors.right: undefined
                anchors.bottom: undefined
                anchors.horizontalCenter: iconBox.horizontalCenter
                anchors.verticalCenter: undefined
            }
        },
        State {
            name: "left"
            when: Plasmoid.location === LingmoCore.Types.LeftEdge
            AnchorChanges {
                target: arrow
                anchors.top: undefined
                anchors.left: arrow.parent.left
                anchors.right: undefined
                anchors.bottom: undefined
                anchors.horizontalCenter: undefined
                anchors.verticalCenter: iconBox.verticalCenter
            }
        },
        State {
            name: "right"
            when: Plasmoid.location === LingmoCore.Types.RightEdge
            AnchorChanges {
                target: arrow
                anchors.top: undefined
                anchors.left: undefined
                anchors.right: arrow.parent.right
                anchors.bottom: undefined
                anchors.horizontalCenter: undefined
                anchors.verticalCenter: iconBox.verticalCenter
            }
        }
    ]

    implicitWidth: Math.min(naturalSize.width, iconBox.width)
    implicitHeight: Math.min(naturalSize.height, iconBox.width)

    imagePath: "widgets/tasks"
    elementId: elementForLocation()

    function elementForLocation() {
        switch (Plasmoid.location) {
            case LingmoCore.Types.LeftEdge:
                return "group-expander-left";
            case LingmoCore.Types.TopEdge:
                return "group-expander-top";
            case LingmoCore.Types.RightEdge:
                return "group-expander-right";
            case LingmoCore.Types.BottomEdge:
            default:
                return "group-expander-bottom";
        }
    }
}
