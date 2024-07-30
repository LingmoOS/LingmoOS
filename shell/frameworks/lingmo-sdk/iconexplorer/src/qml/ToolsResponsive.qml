/*
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.lingmoui 2.8 as LingmoUI

QQC2.ToolBar {
    id: root

    property alias currentIndex: colorcombo.currentIndex
    property alias value: sizeslider.value

    signal colorschemeChanged(int index)
    signal sliderValueChanged(int val)

    position: QQC2.ToolBar.Footer

    RowLayout {
        anchors.fill: parent
        spacing: LingmoUI.Units.largeSpacing
        QQC2.Slider {
            visible: !cuttlefish.widescreen
            id: sizeslider
            Layout.fillWidth: true

            to: 6.0
            stepSize: 1.0
            snapMode: QQC2.Slider.SnapAlways
            value: 4.0

            onValueChanged: {
                sizetimer.restart()
                pixelSizeInput.text = cuttlefish.iconSizes[sizeslider.value];
                root.sliderValueChanged(sizeslider.value)
            }

            Timer {
                id: sizetimer
                running: false
                repeat: false
                interval: 200
                onTriggered: iconSize = cuttlefish.iconSizes[sizeslider.value];
            }

            Component.onCompleted: {
                pixelSizeInput.text = cuttlefish.iconSizes[sizeslider.value];
            }
        }

        QQC2.Label {
            visible: !cuttlefish.widescreen
            id: pixelSizeInput

            Layout.preferredWidth: LingmoUI.Units.gridUnit
        }
        QQC2.ComboBox {
            id: colorcombo
            visible: !cuttlefish.widescreen
            model: colorSchemes.colorSchemes
            delegate: QQC2.ItemDelegate {
                LingmoUI.Theme.colorSet: LingmoUI.Theme.View
                width: parent.width
                highlighted: colorcombo.highlightedIndex == index
                contentItem: RowLayout {
                    LingmoUI.Icon {
                        source: model.decoration
                        Layout.preferredHeight: LingmoUI.Units.iconSizes.small
                        Layout.preferredWidth: LingmoUI.Units.iconSizes.small
                    }
                    QQC2.Label {
                        text: model.display
                        color: highlighted ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
                        Layout.fillWidth: true
                    }
                }
            }
            textRole: "display"
            onActivated: (index) => {
                root.colorschemeChanged(index)
                colorSchemes.activateColorScheme(currentText)
            }
        }
    }
}
