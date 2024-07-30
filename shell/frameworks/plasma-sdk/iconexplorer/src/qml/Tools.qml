/*
    SPDX-FileCopyrightText: 2014-2017 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.kirigami 2.8 as Kirigami

QQC2.ToolBar {
    id: root

    signal colorschemeChanged(int index)
    signal sliderValueChanged(int val)

    property alias slider: sizeslider
    property alias currentIndex: colorcombo.currentIndex
    property alias value: sizeslider.value

    RowLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing
        Kirigami.SearchField {
            id: filterInput
            Layout.fillWidth: true
            onTextChanged: typingtimer.restart()
            focus: true

            Timer {
                id: typingtimer
                running: false
                repeat: false
                interval: 100
                onTriggered: {
                    proxyModel.filter = filterInput.text
                }
            }
            Component.onCompleted: {
                filterInput.forceActiveFocus()
            }
        }

        QQC2.ComboBox {
            id: catsCombo
            Layout.preferredWidth: Kirigami.Units.gridUnit * 6
            model: iconModel.categories
            onActivated: {
                if (currentText == "all") {
                    proxyModel.category = "";
                } else if (currentText != "") {
                    proxyModel.category = currentText
                }
            }
        }

        QQC2.Slider {
            visible: cuttlefish.widescreen
            id: sizeslider
            Layout.preferredWidth: preview.width - Kirigami.Units.gridUnit * 2

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
            visible: cuttlefish.widescreen
            id: pixelSizeInput

            Layout.preferredWidth: Kirigami.Units.gridUnit * 3
        }
        QQC2.Label {
            visible: cuttlefish.widescreen
            text: i18n("Color scheme:")
        }
        QQC2.ComboBox {
            id: colorcombo
            visible: cuttlefish.widescreen
            model: colorSchemes.colorSchemes
            delegate: QQC2.ItemDelegate {
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                width: parent.width
                highlighted: colorcombo.highlightedIndex == index
                contentItem: RowLayout {
                    Kirigami.Icon {
                        source: model.decoration
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    }
                    QQC2.Label {
                        text: model.display
                        color: highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
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

