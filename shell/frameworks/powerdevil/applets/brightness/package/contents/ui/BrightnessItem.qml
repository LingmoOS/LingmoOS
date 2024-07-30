/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013, 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmoui as LingmoUI

LingmoComponents3.ItemDelegate {
    id: root

    enum Type {
        Screen,
        Keyboard
    }

    property alias slider: control
    property alias value: control.value
    property alias maximumValue: control.to
    property alias stepSize: control.stepSize
    required property /*BrightnessItem.Type*/ int type

    readonly property real percentage: Math.round(100 * value / maximumValue)
    readonly property string brightnessLevelOff: i18nc("Backlight on or off", "Off")
    readonly property string brightnessLevelLow: i18nc("Brightness level", "Low")
    readonly property string brightnessLevelMedium: i18nc("Brightness level", "Medium")
    readonly property string brightnessLevelHigh: i18nc("Brightness level", "High")
    readonly property string brightnessLevelOn: i18nc("Backlight on or off", "On")
    readonly property string labelText: {
        if (maximumValue == 1) {
            const levels = [brightnessLevelOff, brightnessLevelOn];
            return levels[value];
        } else if (maximumValue == 2) {
            const levels = [brightnessLevelOff, brightnessLevelLow, brightnessLevelHigh];
            return levels[value];
        } else if (maximumValue == 3) {
            const levels = [brightnessLevelOff, brightnessLevelLow, brightnessLevelMedium, brightnessLevelHigh];
            return levels[value];
        } else {
            return i18nc("Placeholder is brightness percentage", "%1%", percentage);
        }
    }

    signal moved()

    background.visible: highlighted
    highlighted: activeFocus
    hoverEnabled: false

    Accessible.ignored: true
    Keys.forwardTo: [slider]

    contentItem: RowLayout {
        spacing: LingmoUI.Units.gridUnit

        LingmoUI.Icon {
            id: image
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
            Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
            source: root.icon.name
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                spacing: LingmoUI.Units.smallSpacing

                LingmoComponents3.Label {
                    id: title
                    Layout.fillWidth: true
                    text: root.text
                    textFormat: Text.PlainText
                    Accessible.ignored: true
                }

                LingmoComponents3.Label {
                    id: percent
                    Layout.alignment: Qt.AlignRight
                    text: root.labelText
                    textFormat: Text.PlainText
                    Accessible.ignored: true
                }
            }

            LingmoComponents3.Slider {
                id: control
                Layout.fillWidth: true

                activeFocusOnTab: false
                // Don't allow the slider to turn off the screen
                // Please see https://git.reviewboard.kde.org/r/122505/ for more information
                from: root.type == BrightnessItem.Type.Screen ? 1 : 0
                stepSize: 1

                Accessible.name: root.text
                Accessible.description: percent.text
                Accessible.onPressAction: moved()

                onMoved: root.moved()
            }
        }
    }
}
