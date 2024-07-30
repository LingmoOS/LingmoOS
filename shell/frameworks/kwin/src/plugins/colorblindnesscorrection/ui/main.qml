/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

import org.kde.kcmutils as KCM
import org.kde.lingmo.kwin.colorblindnesscorrectioneffect.kcm

KCM.SimpleKCM {
    id: root

    implicitWidth: LingmoUI.Units.gridUnit * 30
    implicitHeight: LingmoUI.Units.gridUnit * 24

    RowLayout {
        id: previewArea
        Layout.fillWidth: true
        spacing: LingmoUI.Units.smallSpacing

        Item {
            Layout.fillWidth: true
        }

        Repeater {
            model: [
                { name: i18nd("kwin", "Red"), colors: ["Red", "Orange", "Yellow"] },
                { name: i18nd("kwin", "Green"), colors: ["Green", "LimeGreen", "Lime"] },
                { name: i18nd("kwin", "Blue"), colors: ["Blue", "DeepSkyBlue", "Aqua"] },
                { name: i18nd("kwin", "Purple"), colors: ["Purple", "Fuchsia", "Violet"] },
            ]

            delegate: Column {
                spacing: 0

                Repeater {
                    model: modelData.colors
                    delegate: Rectangle {
                        width: LingmoUI.Units.gridUnit * 5
                        height: LingmoUI.Units.gridUnit * 5
                        color: modelData
                    }
                }

                QQC.Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: modelData.name
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    LingmoUI.FormLayout {
        id: formLayout
        anchors {
            top: previewArea.bottom
            topMargin: LingmoUI.Units.largeSpacing
        }

        QQC.ComboBox {
            id: colorComboBox
            LingmoUI.FormData.label: i18ndc("kwin", "@label", "Mode:")
            currentIndex: kcm.settings.mode
            textRole: "text"
            valueRole: "value"
            model: [
                { value: 0, text: i18ndc("kwin", "@option", "Protanopia (red weak)") },
                { value: 1, text: i18ndc("kwin", "@option", "Deuteranopia (green weak)") },
                { value: 2, text: i18ndc("kwin", "@option", "Tritanopia (blue-yellow)") },
            ]

            onActivated: kcm.settings.mode = currentValue
        }

        QQC.Slider {
            LingmoUI.FormData.label: i18ndc("kwin", "@label", "Intensity:")
            Layout.preferredWidth: colorComboBox.implicitWidth
            from: 0.0
            to: 1.0
            value: kcm.settings.intensity

            onMoved: kcm.settings.intensity = value
        }
    }
}
