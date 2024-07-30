/*
    SPDX-FileCopyrightText: 2014-2017 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2020 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.kirigami 2.8 as Kirigami

Rectangle {
    id: root
    property int columns: cuttlefish.iconSizes.length
    property bool showWatermark: false

    color: Kirigami.Theme.backgroundColor

    implicitWidth: previewGridLayout.implicitWidth
    implicitHeight: previewGridLayout.implicitHeight

    GridLayout {
        id: previewGridLayout

        anchors.centerIn: parent

        Repeater {
            model: cuttlefish.iconSizes
            delegate: ColumnLayout {
                readonly property bool isLastItem: (index == columns)
                Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                Layout.row: isLastItem ? 1 : 0
                Layout.column: isLastItem ? 0 : index
                Layout.columnSpan: isLastItem ? columns : 1
                Kirigami.Icon {
                    source: preview.iconName
                    implicitWidth: modelData
                    implicitHeight: implicitWidth
                }
                QQC2.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: modelData
                    // Inheriting doesn't correctly work when in a repeater
                    color: root.Kirigami.Theme.textColor
                    Behavior on color {
                        ColorAnimation {
                            duration: Kirigami.Units.longDuration
                        }
                    }
                }
            }
        }
    }

    Row {
        opacity: showWatermark ? 1 : 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: Kirigami.Units.smallSpacing
        Kirigami.Icon {
            height: 32
            width: 32
            source: "cuttlefish"
        }
        QQC2.Label {
            anchors.verticalCenter: parent.verticalCenter
            text: "Montage made with Cuttlefish"
        }
        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.longDuration
            }
        }
    }
}
