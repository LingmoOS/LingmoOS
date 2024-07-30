/*
    SPDX-FileCopyrightText: 2020 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.draganddrop 2.0 as DND
import org.kde.lingmo.extras 2.0 as LingmoExtras

DND.DropArea {
    id: root

    signal taskDropped(variant mimeData, variant modifiers)
    signal clicked()
    signal entered()

    property int topPadding: 0
    property string activityName: ""
    property bool selected: false
    property string actionTitle: ""
    property bool isHovered: false
    property bool actionVisible: false

    LingmoExtras.Highlight {
        id: dropHighlight
        anchors {
            fill: parent
            // topMargin: icon.height + 3 * LingmoUI.Units.smallSpacing
            topMargin: root.topPadding
        }
        visible: root.isHovered
        z: -1
    }

    Text {
        id: dropAreaLeftText
        anchors {
            fill: dropHighlight
            leftMargin: LingmoUI.Units.gridUnit
            rightMargin: LingmoUI.Units.gridUnit
        }

        color: LingmoUI.Theme.textColor
        visible: root.actionVisible

        text: root.actionTitle
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        maximumLineCount: 3
    }

    anchors {
        left: parent.left
        right: parent.horizontalCenter
        top: parent.top
        bottom: parent.bottom
    }

    preventStealing: true
    enabled: true

    onDrop: {
        root.taskDropped(event.mimeData, event.modifiers);
    }

    onDragEnter: {
        root.isHovered = true;
    }

    onDragLeave: {
        root.isHovered = false;
    }

    MouseArea {
        anchors.fill : parent
        onClicked    : root.clicked()
        hoverEnabled : true
        onEntered    : root.entered()

        Accessible.name          : root.activityName
        Accessible.role          : Accessible.Button
        Accessible.selected      : root.selected
        Accessible.onPressAction : root.clicked()
    }
}
