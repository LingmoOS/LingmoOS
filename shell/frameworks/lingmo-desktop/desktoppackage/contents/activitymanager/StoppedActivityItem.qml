/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.kcmutils  // KCMLauncher
import org.kde.config  // KAuthorized

import org.kde.lingmo.activityswitcher as ActivitySwitcher

import "static.js" as S

Item {
    id: root

    property int innerPadding: LingmoUI.Units.smallSpacing

    property string activityId : ""

    property alias title       : title.text
    property alias icon        : icon.source

    signal clicked

    width  : 200
    height : icon.height + 2 * LingmoUI.Units.smallSpacing

    // Background until we get something real
    KSvg.FrameSvgItem {
        id: highlight
        imagePath: "widgets/viewitem"
        visible: rootArea.containsMouse

        anchors {
            fill: parent
            rightMargin: -highlight.margins.right
            bottomMargin: -highlight.margins.bottom
        }

        prefix: "normal"
    }

    Item {
        anchors {
            fill: parent

            leftMargin: highlight.margins.left
            topMargin: highlight.margins.top
        }

        // Title and the icon

        LingmoUI.Icon {
            id: icon

            width  : LingmoUI.Units.iconSizes.medium
            height : width

            anchors {
                left    : parent.left
                margins : root.innerPadding
                verticalCenter: parent.verticalCenter
            }
        }

        LingmoComponents.Label {
            id: title

            elide : Text.ElideRight

            anchors {
                left    : icon.right
                right   : parent.right
                margins : root.innerPadding * 2
                verticalCenter: parent.verticalCenter
            }
            textFormat: Text.PlainText
        }

        // Controls

        MouseArea {
            id: rootArea

            anchors.fill : parent
            hoverEnabled : true

            Accessible.name          : root.title
            Accessible.role          : Accessible.Button
            Accessible.selectable    : false
            Accessible.onPressAction : root.clicked()

            onClicked    : root.clicked()
            onEntered    : S.showActivityItemActionsBar(root)
        }

        Item {
            id: controlBar

            height: root.state == "showingControls" ?
                        root.height :
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
                left   : parent.left
                right  : parent.right
                bottom : parent.bottom
            }

            LingmoComponents.Button {
                id: configButton

                icon.name: "configure"
                LingmoComponents.ToolTip.delay: LingmoUI.Units.toolTipDelay
                LingmoComponents.ToolTip.visible: hovered
                LingmoComponents.ToolTip.text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Configure activity")

                onClicked: KCMLauncher.openSystemSettings("kcm_activities", root.activityId)

                anchors {
                    right       : deleteButton.left
                    rightMargin : 2 * LingmoUI.Units.smallSpacing
                    verticalCenter: parent.verticalCenter
                }
            }

            LingmoComponents.Button {
                id: deleteButton

                icon.name: "edit-delete"
                LingmoComponents.ToolTip.delay: LingmoUI.Units.toolTipDelay
                LingmoComponents.ToolTip.visible: hovered
                LingmoComponents.ToolTip.text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Delete")

                onClicked: ActivitySwitcher.Backend.removeActivity(root.activityId)
                visible: KAuthorized.authorize("lingmo-desktop/add_activities")

                anchors {
                    right       : parent.right
                    rightMargin : 2 * LingmoUI.Units.smallSpacing + 2
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    states: [
        State {
            name: "plain"
            PropertyChanges { target: controlBar; opacity: 0 }
        },
        State {
            name: "showingControls"
            PropertyChanges { target: controlBar; opacity: 1 }
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


