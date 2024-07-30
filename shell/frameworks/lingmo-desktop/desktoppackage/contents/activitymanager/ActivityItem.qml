/*
    SPDX-FileCopyrightText: 2014-2020 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.activityswitcher as ActivitySwitcher
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.kcmutils  // KCMLauncher

import "static.js" as S

Item {
    id: root

    property int innerPadding  : LingmoUI.Units.gridUnit

    property bool current      : false
    property bool selected     : false
    property bool stoppable    : true

    property alias title       : title.text
    property alias icon        : icon.source
    property alias hasWindows  : hasWindowsIndicator.visible

    z : current  ? 10 :
        selected ?  5 : 0

    property string activityId : ""

    property string background : ""

    onBackgroundChanged: if (background[0] !== '#') {
        // We have a proper wallpaper, hurroo!
        backgroundColor.visible = false;

    } else {
        // We have only a color
        backgroundColor.color = background;
        backgroundColor.visible = true;
    }

    signal clicked

    width  : 200
    height : width * 9.0 / 16.0

    Item {
        anchors {
            fill: parent
        }

        // Background until we get something real
        Rectangle {
            id: backgroundColor

            anchors.fill: parent
            // This is intentional - while waiting for the wallpaper,
            // we are showing a semi-transparent black background
            color: "black"

            opacity: root.selected ? .8 : .5
        }

        Image {
            id: backgroundWallpaper

            anchors.fill: parent

            visible: !backgroundColor.visible
            source: "image://wallpaperthumbnail/" + background
            sourceSize: Qt.size(width, height)
        }

        // Title and the icon

        Rectangle {
            id: shade

            width: parent.height
            height: parent.width

            anchors.centerIn: parent
            rotation: 90

            gradient: Gradient {
                GradientStop { position: 1.0; color: "black" }
                GradientStop { position: 0.0; color: "transparent" }
            }

            opacity : root.selected ? 0.5 : 1.0
        }

        Rectangle {
            id: currentActivityHighlight

            visible:  root.current

            border.width: root.current ? LingmoUI.Units.smallSpacing : 0
            border.color: LingmoUI.Theme.highlightColor

            z: 10

            anchors {
                fill: parent
                // Hide the rounding error on the bottom of the rectangle
                bottomMargin: -1
            }

            color: "transparent"
        }

        Item {
            id: titleBar

            anchors {
                top   : parent.top
                left  : parent.left
                right : parent.right

                leftMargin : 2 * LingmoUI.Units.smallSpacing + 2
                topMargin  : 2 * LingmoUI.Units.smallSpacing
            }

            Text {
                id: title

                color   : "white"
                elide   : Text.ElideRight
                visible : shade.visible

                font.bold : true

                anchors {
                    top   : parent.top
                    left  : parent.left
                    right : icon.left
                }
            }

            Text {
                id: description

                color   : "white"
                elide   : Text.ElideRight
                text    : model.description
                opacity : .6

                anchors {
                    top   : title.bottom
                    left  : parent.left
                    right : icon.left
                }
            }

            LingmoUI.Icon {
                id: icon

                width   : LingmoUI.Units.iconSizes.medium
                height  : width

                anchors {
                    right       : parent.right
                    rightMargin : 2 * LingmoUI.Units.smallSpacing
                }
            }
        }

        Column {
            id: statsBar

            height: childrenRect.height + LingmoUI.Units.smallSpacing

            anchors {
                bottom : controlBar.top
                left   : parent.left
                right  : parent.right

                leftMargin   : 2 * LingmoUI.Units.smallSpacing + 2
                rightMargin  : 2 * LingmoUI.Units.smallSpacing
                bottomMargin : LingmoUI.Units.smallSpacing
            }

            LingmoUI.Icon {
                id      : hasWindowsIndicator
                source  : "window-duplicate"
                width   : 16
                height  : width
                opacity : .6
                visible : false
            }

            Text {
                id: lastUsedDate

                color   : "white"
                elide   : Text.ElideRight
                opacity : .6

                text: root.current ?
                        i18nd("lingmo_shell_org.kde.lingmo.desktop", "Currently being used") :
                        model.lastTimeUsedString
            }
        }

        Rectangle {
            id: dropHighlight
            visible: moveDropAction.isHovered || copyDropAction.isHovered

            onVisibleChanged: {
                ActivitySwitcher.Backend.setDropMode(visible);
                if (visible) {
                    root.state = "dropAreasShown";
                } else {
                    root.state = "plain";
                }
            }

            anchors {
                fill: parent
                topMargin: icon.height + 3 * LingmoUI.Units.smallSpacing
            }

            opacity: .75
            color: LingmoUI.Theme.backgroundColor
        }

        TaskDropArea {
            id: moveDropAction

            anchors {
                right: parent.horizontalCenter
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }

            topPadding: icon.height + 3 * LingmoUI.Units.smallSpacing
            actionVisible: dropHighlight.visible

            actionTitle: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Move to\nthis activity")

            onTaskDropped: {
                ActivitySwitcher.Backend.dropMove(mimeData, root.activityId);
            }

            onClicked: {
                root.clicked();
            }

            onEntered: {
                S.showActivityItemActionsBar(root);
            }

            visible: ActivitySwitcher.Backend.dropEnabled
        }

        TaskDropArea {
            id: copyDropAction

            topPadding: icon.height + 3 * LingmoUI.Units.smallSpacing
            actionVisible: dropHighlight.visible

            anchors {
                right: parent.right
                left: parent.horizontalCenter
                top: parent.top
                bottom: parent.bottom
            }

            actionTitle: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Show also\nin this activity")

            onTaskDropped: {
                ActivitySwitcher.Backend.dropCopy(mimeData, root.activityId);
            }

            onClicked: {
                root.clicked();
            }

            onEntered: {
                S.showActivityItemActionsBar(root);
            }

            visible: ActivitySwitcher.Backend.dropEnabled

        }

        // Controls
        Item {
            id: controlBar

            height: root.state == "showingControls" ?
                        (configButton.height + 4 * LingmoUI.Units.smallSpacing) :
                        0

            Behavior on height {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: LingmoUI.Units.shortDuration
                }
            }

            clip: true

            anchors {
                bottom : parent.bottom
                left   : parent.left
                right  : parent.right
            }

            Rectangle {
                anchors {
                    fill: parent
                    margins: - 2 * LingmoUI.Units.smallSpacing
                }

                opacity: .75
                color: LingmoUI.Theme.backgroundColor
            }

            LingmoComponents.Button {
                id: configButton

                icon.name: "configure"
                LingmoComponents.ToolTip.delay: LingmoUI.Units.toolTipDelay
                LingmoComponents.ToolTip.visible: hovered
                LingmoComponents.ToolTip.text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Configure")

                onClicked: KCMLauncher.openSystemSettings("kcm_activities", root.activityId);

                anchors {
                    left       : parent.left
                    top        : parent.top
                    leftMargin : 2 * LingmoUI.Units.smallSpacing + 2
                    topMargin  : 2 * LingmoUI.Units.smallSpacing
                }
            }

            LingmoComponents.Button {
                id: stopButton

                visible: stoppable
                icon.name: "process-stop"
                LingmoComponents.ToolTip.delay: LingmoUI.Units.toolTipDelay
                LingmoComponents.ToolTip.visible: hovered
                LingmoComponents.ToolTip.text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Stop activity")

                onClicked: ActivitySwitcher.Backend.stopActivity(activityId);

                anchors {
                    right       : parent.right
                    top         : parent.top
                    rightMargin : 2 * LingmoUI.Units.smallSpacing + 2
                    topMargin   : 2 * LingmoUI.Units.smallSpacing
                }
            }
        }
    }

    states: [
        State {
            name: "plain"
            PropertyChanges { target: shade; visible: true }
            PropertyChanges { target: controlBar; opacity: 0 }
        },
        State {
            name: "showingControls"
            PropertyChanges { target: shade; visible: true }
            PropertyChanges { target: controlBar; opacity: 1 }
        },
        State {
            name: "dropAreasShown"
            // PropertyChanges { target: shade; visible: false }
            PropertyChanges { target: statsBar; visible: false }
            PropertyChanges { target: controlBar; opacity: 0 }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties : "opacity"
                duration   : LingmoUI.Units.shortDuration
            }
        }
    ]
}
