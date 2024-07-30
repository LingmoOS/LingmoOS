/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.ksvg as KSvg
import org.kde.lingmo.components as PC3

import org.kde.lingmo.welcome

Item {
    id: root

    implicitWidth: LingmoUI.Units.gridUnit * 16 + LingmoUI.Units.smallSpacing * 2 // Matches width of Activities side panel

    KSvg.FrameSvgItem {
        anchors.fill: background

        anchors.topMargin: -margins.top
        anchors.leftMargin: -margins.left
        anchors.rightMargin: -margins.right
        anchors.bottomMargin: -margins.bottom

        imagePath: "dialogs/background"
        prefix: "shadow"
    }

    KSvg.FrameSvgItem {
        id: background
        anchors.fill: parent

        imagePath: "dialogs/background"
        enabledBorders: KSvg.FrameSvg.NoBorder
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.smallSpacing

        spacing: 0

        RowLayout {
            Layout.fillHeight: false
            Layout.leftMargin: LingmoUI.Units.smallSpacing
            Layout.rightMargin: LingmoUI.Units.smallSpacing

            LingmoUI.Heading {
                Layout.fillWidth: true
                Layout.fillHeight: true

                level: 1
                text: i18nc("@title:window Title of the Lingmo activities sidebar", "Activities")
            }

            LingmoUI.Icon {
                Layout.margins: LingmoUI.Units.smallSpacing

                implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                implicitWidth: LingmoUI.Units.iconSizes.smallMedium

                source: "edit-find"
            }

            LingmoUI.Icon {
                Layout.margins: LingmoUI.Units.smallSpacing

                implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                implicitWidth: LingmoUI.Units.iconSizes.smallMedium

                source: "configure"
            }

            LingmoUI.Icon {
                Layout.margins: LingmoUI.Units.smallSpacing

                implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                implicitWidth: LingmoUI.Units.iconSizes.smallMedium

                source: "window-close"
            }
        }

        Repeater {
            model: [ { name: i18nc("@title The name of an example Lingmo activity", "Home"),   active: true  },
                     { name: i18nc("@title The name of an example Lingmo activity", "School"), active: false },
                     { name: i18nc("@title The name of an example Lingmo activity", "Work"),   active: false }  ]
            delegate: Item {
                id: activity
                Layout.fillWidth: true
                Layout.topMargin: LingmoUI.Units.smallSpacing * 2
                Layout.rightMargin: LingmoUI.Units.smallSpacing

                required property string name
                required property bool active

                implicitHeight: width * 9.0 / 16.0 // Match aspect of the wallpaper source (see below)

                Image {
                    anchors.fill: parent
                    // We're intentionally ignoring our 16:10 desktop wallpaper (MockDesktop) - whilst that fits the square
                    // window, we want to maximise what we can fit in our limited height and be more representative of what
                    // a user will  see - most use a 16:9 display.
                    source: "file:" + Controller.installPrefix() + "/share/wallpapers/Next/contents/images/1920x1080.png"
                    mipmap: true
                }

                Rectangle {
                    anchors.centerIn: parent

                    width: parent.height
                    height: parent.width
                    rotation: 90

                    gradient: Gradient {
                        GradientStop { position: 1.0; color: "black" }
                        GradientStop { position: 0.0; color: "transparent" }
                    }

                    opacity : activity.active ? 0.5 : 1.0
                }

                Rectangle {
                    anchors.fill: parent

                    visible: activity.active
                    color: "transparent"
                    border.width: activity.active ? LingmoUI.Units.smallSpacing : 0
                    border.color: LingmoUI.Theme.highlightColor
                }

                Text {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: icon.left
                    anchors.topMargin: LingmoUI.Units.smallSpacing * 2
                    anchors.leftMargin: (LingmoUI.Units.smallSpacing * 2) + 2

                    text: activity.name
                    elide: Text.ElideRight
                    font.bold: true
                    color: "white"
                }

                LingmoUI.Icon {
                    id: icon
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.topMargin: LingmoUI.Units.smallSpacing * 2
                    anchors.rightMargin: LingmoUI.Units.smallSpacing * 2

                    source: "activities-symbolic"
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: (LingmoUI.Units.smallSpacing * 2) + 2
                    anchors.rightMargin: LingmoUI.Units.smallSpacing * 2
                    anchors.bottomMargin: LingmoUI.Units.smallSpacing * 2

                    text: activity.active ? i18nc("@info:status An example last used message for a Lingmo activity", "Currently being used")
                                          : i18nc("@info:status An example last used message for a Lingmo activity", "Used 3 hours ago")
                    elide: Text.ElideRight
                    color: "white"
                    opacity: 0.6
                }
            }
        }

        Item {
            id: createButtonSpacing
            Layout.fillHeight: true
        }

        PC3.ToolButton {
            Layout.fillWidth: true
            Layout.bottomMargin: LingmoUI.Units.gridUnit

            icon.name: "list-add"
            text: i18nc("@action:button Button in the Lingmo activities sidebar", "Create activity…")

            // Hide when there's not enough space
            opacity: createButtonSpacing.height < LingmoUI.Units.smallSpacing * 2 ? 0 : 1

            Behavior on opacity {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            focusPolicy: Qt.NoFocus
        }
    }

    // Eat mouse events (hover, click, scroll)
    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.AllButtons
        hoverEnabled: true
    }
}
