// SPDX-FileCopyrightText: 2020-2024 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.workspace.keyboardlayout 1.0
import org.kde.lingmo.private.mobileshell as MobileShell

import org.kde.lingmoui 2.12 as LingmoUI

Item {
    id: root
    required property real openProgress
    required property var lockScreenState

    property alias passwordBar: passwordBar

    MobileShell.HapticsEffect {
        id: haptics
    }

    // Column layout - most cases
    ColumnLayout {
        id: keypadVerticalContainer
        visible: root.height > LingmoUI.Units.gridUnit * 25

        anchors.centerIn: parent
        spacing: LingmoUI.Units.gridUnit * 2

        LayoutItemProxy { target: header }
        LayoutItemProxy { target: keypadGrid }
    }

    // Row layout - used when there is restricted height
    RowLayout {
        id: keypadHorizontalContainer
        visible: !keypadVerticalContainer.visible

        anchors.centerIn: parent
        spacing: LingmoUI.Units.gridUnit * 2

        LayoutItemProxy { target: header }
        LayoutItemProxy { target: keypadGrid }
    }

    ColumnLayout {
        id: header
        spacing: LingmoUI.Units.gridUnit

        // label ("wrong pin", "enter pin")
        Label {
            id: descriptionLabel
            Layout.alignment: Qt.AlignHCenter
            opacity: root.lockScreenState.password.length === 0 ? 1 : 0
            text: root.lockScreenState.pinLabel
            font.pointSize: 12
            font.bold: true
            color: 'white'

            // Enforce extra margin at top of vertical container
            Layout.topMargin: keypadVerticalContainer.visible ? LingmoUI.Units.gridUnit * 3 : 0

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
        }

        // pin display and bar
        PasswordBar {
            id: passwordBar
            Layout.preferredWidth: LingmoUI.Units.gridUnit * 14
            Layout.preferredHeight: LingmoUI.Units.gridUnit * 2.5

            lockScreenState: root.lockScreenState
            isKeypadOpen: root.openProgress >= 0.9
        }
    }

    GridLayout {
        id: keypadGrid
        columnSpacing: LingmoUI.Units.gridUnit
        rowSpacing: LingmoUI.Units.gridUnit
        uniformCellHeights: true
        uniformCellWidths: true

        readonly property real intendedWidth: LingmoUI.Units.gridUnit * 14

        Layout.preferredWidth: LingmoUI.Units.gridUnit * 14
        Layout.preferredHeight: LingmoUI.Units.gridUnit * 22

        readonly property real cellLength: (intendedWidth - columnSpacing * 2) / 3

        columns: 3

        // numpad keys
        Repeater {
            model: ["1", "2", "3", "4", "5", "6", "7", "8", "9", "R", "0", "E"]

            delegate: AbstractButton {
                id: button
                implicitWidth: keypadGrid.cellLength
                implicitHeight: keypadGrid.cellLength
                visible: modelData.length > 0
                enabled: root.openProgress >= 0.8 // Only enable after a certain point in animation

                opacity: enabled
                Behavior on opacity {
                    SequentialAnimation {
                        PauseAnimation { duration: 20 * index }
                        NumberAnimation { duration: 300 }
                    }
                }

                background: Rectangle {
                    readonly property real restingOpacity: (modelData !== "R" && modelData !== "E") ? 0.2 : 0.0
                    radius: width
                    color: Qt.rgba(255, 255, 255,
                                    button.pressed ? 0.5 : restingOpacity)
                }

                onPressedChanged: {
                    if (pressed) {
                        haptics.buttonVibrate();
                    }
                }

                onClicked: {
                    if (modelData === "R") {
                        passwordBar.backspace();
                    } else if (modelData === "E") {
                        passwordBar.enter();
                    } else {
                        passwordBar.keyPress(modelData);
                    }
                }

                onPressAndHold: {
                    if (modelData === "R") {
                        haptics.buttonVibrate();
                        passwordBar.clear();
                    }
                }

                contentItem: Item {
                    LingmoComponents.Label {
                        visible: modelData !== "R" && modelData !== "E"
                        text: modelData
                        anchors.centerIn: parent
                        font.pointSize: 18
                        color: 'white'
                    }

                    LingmoUI.Icon {
                        visible: modelData === "R" || modelData === "E"
                        anchors.centerIn: parent
                        width: LingmoUI.Units.iconSizes.small
                        height: LingmoUI.Units.iconSizes.small
                        source: modelData === "R" ? "edit-clear" : "go-next"
                        LingmoUI.Theme.inherit: false
                        LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary
                    }
                }
            }
        }
    }
}
