// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0

import org.kde.kirigami 2.7 as Kirigami

import PicoWizard 1.0

FocusScope {
    property alias moduleName: labelModuleName.text
    property alias moduleIcon: icon.source
    property alias delegate: delegateLoader.sourceComponent
    property bool hideSkip: false

    Pane {
        id: control
        anchors {
            fill: parent
            leftMargin: 32
            rightMargin: 32
            topMargin: 32
            bottomMargin: 32
        }
        background: Rectangle {
            color: Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 0.85)
            radius: 4
            border.width: 1
            border.color: Qt.lighter(Kirigami.Theme.backgroundColor, 1.5)
        }

        ColumnLayout {
            anchors.fill: parent

            RowLayout {
                Layout.alignment: Layout.Center

                Kirigami.Icon {
                    id: icon
                    isMask: true
                    color: Qt.lighter(Kirigami.Theme.disabledTextColor, 1.5)
                    opacity: 0.8
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    Layout.alignment: Layout.Center
                    Layout.topMargin: 12
                    Layout.bottomMargin: 12
                }
                Label {
                    id: labelModuleName
                    font.weight: Font.Light
                    font.pointSize: Kirigami.Settings.isMobile ? 16 : 24
                    color: Kirigami.Theme.textColor
                }
            }

            Loader {
                id: delegateLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
