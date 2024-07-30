// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0

import org.kde.kirigami 2.7 as Kirigami

import PicoWizard 1.0

ColumnLayout {
    property alias moduleName: labelModuleName.text
    property alias moduleIcon: icon.source
    property alias delegate: delegateLoader.sourceComponent
    property bool hideSkip: false

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: false
        Layout.topMargin: 64
        Layout.rightMargin: Kirigami.Settings.isMobile ? 24 : 56

        RoundButton {
            Layout.fillWidth: false
            Layout.fillHeight: false
            Layout.preferredWidth: Kirigami.Settings.isMobile ? 40 : 48
            Layout.preferredHeight: Kirigami.Settings.isMobile ? 40 : 48
            hoverEnabled: false

            flat: true
            onClicked: {
                moduleLoader.back()
            }
            opacity: moduleLoader.hasPrevious ? 1 : 0
            z: 100

            background: Rectangle {
                color: "transparent"
            }

            Kirigami.Icon {
                anchors.fill: parent
                anchors.centerIn: parent
                source: "go-previous"
                anchors.margins: Kirigami.Units.largeSpacing
                color: Kirigami.Theme.highlightedTextColor
                isMask: true
            }
        }

        Label {
            id: labelModuleName
            Layout.fillWidth: true
            Layout.fillHeight: false
            font.weight: Font.Light
            font.pointSize: Kirigami.Settings.isMobile ? 16 : 24
            color: Kirigami.Theme.highlightedTextColor
        }

        Label {
            id: labelSkip
            Layout.fillWidth: false
            Layout.fillHeight: false
            font.weight: Font.Light
            font.pointSize: Kirigami.Settings.isMobile ? 16 : 24
            color: Kirigami.Theme.highlightedTextColor
            text: qsTr("Skip")
            visible: !hideSkip

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    moduleLoader.nextModule()
                }
            }
        }
    }

    Kirigami.Page {
        id: control
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.leftMargin: Kirigami.Settings.isMobile ? 24 : 56
        Layout.rightMargin: Kirigami.Settings.isMobile ? 24 : 56
        background: Rectangle {
            color: Qt.lighter(Kirigami.Theme.backgroundColor, 1.1)
            radius: 4
            border.width: 1
            border.color: Qt.darker(Kirigami.Theme.backgroundColor, 1.05)
        }

        ColumnLayout {
            anchors.fill: parent

            Kirigami.Icon {
                id: icon
                isMask: true
                color: Qt.lighter(Kirigami.Theme.disabledTextColor, 1.5)
                opacity: 0.8

                Layout.preferredWidth: Kirigami.Settings.isMobile ? 64 : 196
                Layout.preferredHeight: Kirigami.Settings.isMobile ? 64 : 196
                Layout.alignment: Layout.Center
                Layout.topMargin: Kirigami.Settings.isMobile ? 24 : 64
                Layout.bottomMargin: Kirigami.Settings.isMobile ? 24 : 64
            }

            Loader {
                id: delegateLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
