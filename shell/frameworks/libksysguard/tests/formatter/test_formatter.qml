/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.ksysguard.formatter as Formatter

Pane {
    width: 400
    height: 400

    ColumnLayout {
        anchors.fill: parent

        TextField {
            id: input
            Layout.fillWidth: true
        }

        ComboBox {
            id: unitCombo

            Layout.fillWidth: true

            textRole: "key"

            model: [
                { key: "Bytes", value: Formatter.Units.UnitByte },
                { key: "Kilobytes", value: Formatter.Units.UnitKiloByte },
                { key: "Megabytes", value: Formatter.Units.UnitMegaByte },
                { key: "Bytes per Second", value: Formatter.Units.UnitByteRate },
                { key: "Hertz", value: Formatter.Units.UnitHertz },
                { key: "Second", value: Formatter.Units.UnitSecond },
                { key: "Celcius", value: Formatter.Units.UnitCelsius },
                { key: "Volt", value: Formatter.Units.UnitVolt },
                { key: "Watt", value: Formatter.Units.UnitWatt }
            ]
        }

        ComboBox {
            id: prefixCombo

            Layout.fillWidth: true

            textRole: "key"

            model: [
                { key: "None", value: Formatter.Units.MetricPrefixUnity },
                { key: "Auto-adjust", value: Formatter.Units.MetricPrefixAutoAdjust },
                { key: "Kilo", value: Formatter.Units.MetricPrefixKilo },
                { key: "Mega", value: Formatter.Units.MetricPrefixMega },
                { key: "Giga", value: Formatter.Units.MetricPrefixGiga },
                { key: "Tera", value: Formatter.Units.MetricPrefixTera },
                { key: "Peta", value: Formatter.Units.MetricPrefixPeta }
            ]
        }

        Label {
            text: Formatter.Formatter.formatValueShowNull(input.text,
                                                          unitCombo.model[unitCombo.currentIndex].value,
                                                          prefixCombo.model[prefixCombo.currentIndex].value
                                                         )
        }
    }
}
