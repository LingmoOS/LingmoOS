/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.ksysguard.formatter as Formatter
import org.kde.ksysguard.sensors as Sensors

/**
 * A control to select a value with a unit.
 *
 * This is primarily intended for range selection in Face configuration pages.
 * It allows selecting a value and a unit for that value and provides that
 * value, a unit and a multiplier for that value.
 */
Control {
    id: control

    /**
     * The lower bound for the value.
     */
    property alias from: spinBox.from
    /**
     * The upper bound for the value.
     */
    property alias to: spinBox.to
    /**
     * The value.
     */
    property real value
    /**
     * The unit for the value.
     */
    property int unit
    /**
     * The multiplier to convert the provided value from its unit to the base unit.
     */
    property real multiplier
    /**
     * The list of sensors to use for retrieving unit information.
     */
    property alias sensors: unitModel.sensors
    /**
     * Emitted whenever the value, unit or multiplier changes due to user input.
     */
    signal valueModified()

    implicitWidth: leftPadding + spinBox.implicitWidth + comboBox.implicitWidth + rightPadding
    implicitHeight: topPadding + Math.max(spinBox.implicitHeight, comboBox.implicitHeight) + bottomPadding

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    
    contentItem: RowLayout {
        spacing: 0

        SpinBox {
            id: spinBox

            Layout.fillWidth: true
            Layout.preferredWidth: 0

            editable: true
            from: Math.pow(-2, 31) + 1
            to: Math.pow(2, 31) - 1
            stepSize: 100

            value: control.value * 100

            Binding {
                target: control
                property: "value"
                value: spinBox.value / 100
            }

            validator: DoubleValidator {
                locale: spinBox.locale.name
                bottom: Math.min(spinBox.from, spinBox.to)
                top:  Math.max(spinBox.from, spinBox.to)
            }

            textFromValue: function(value, locale) {
                // "toLocaleString" has no concept of "the minimum amount of
                // digits to represent this number", so we need to calculate this
                // manually. This ensures that things like "0" and "10" will be
                // displayed without any decimals, while things like "2.2" and
                // "3.87" will be displayed with the right number of decimals.

                let realValue = value / 100
                let fract = realValue - Math.trunc(realValue)

                let digits = 0
                if (fract != 0) {
                    digits++;
                }
                if ((fract * 10) - Math.trunc(fract * 10) != 0) {
                    digits++;
                }

                return Number(value / 100).toLocaleString(locale, 'f', digits)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 100
            }

            onValueModified: control.valueModified()
        }

        ComboBox {
            id: comboBox

            Layout.fillWidth: true
            Layout.preferredWidth: 0

            visible: unitModel.sensors.length > 0

            textRole: "symbol"
            valueRole: "unit"

            currentIndex: 0

            onActivated: {
                control.unit = currentValue
                control.multiplier = model.data(model.index(currentIndex, 0), Sensors.SensorUnitModel.MultiplierRole)
                control.valueModified()
            }

            Component.onCompleted: updateCurrentIndex()

            model: Sensors.SensorUnitModel {
                id: unitModel
                onReadyChanged: comboBox.updateCurrentIndex()
            }

            function updateCurrentIndex() {
                if (unitModel.ready && control.unit >= 0) {
                    currentIndex = indexOfValue(control.unit)
                } else {
                    currentIndex = 0;
                }
            }
        }
    }
}
