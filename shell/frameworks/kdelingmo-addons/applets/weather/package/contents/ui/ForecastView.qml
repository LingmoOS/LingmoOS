/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents

GridLayout {
    id: root

    property alias model: repeater.model
    property bool showNightRow: false
    readonly property int preferredIconSize: LingmoUI.Units.iconSizes.large
    readonly property bool hasContent: model && model.length > 0
    readonly property var rowHasProbability: [...Array(rows).keys()].map(
        row => model.filter((_, index) => index % root.rows == row)
                    .some(item => item.probability))

    columnSpacing: LingmoUI.Units.smallSpacing
    rowSpacing: LingmoUI.Units.largeSpacing

    rows: showNightRow ? 2 : 1
    flow: showNightRow ? GridLayout.TopToBottom : GridLayout.LeftToRight

    Repeater {
        id: repeater

        delegate: ColumnLayout {
            id: dayDelegate
            // Allow to set placeholder items by leaving the data empty or setting a label text
            readonly property bool isPlaceHolder: !modelData || modelData.lenght === 0 || !!modelData.placeholder
            readonly property bool isFirstRow: (model.index % root.rows) === 0

            Layout.fillWidth: true
            spacing: Math.round(LingmoUI.Units.smallSpacing / 2)

            LingmoComponents.Label {
                id: periodLabel
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.fillHeight: isPlaceHolder
                // Hide period titles on the second row
                visible: (isPlaceHolder || isFirstRow)

                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                text: isPlaceHolder ? modelData.placeholder || "" : modelData.period.replace(" nt", "")
                textFormat: Text.PlainText
            }

            LingmoUI.Icon {
                Layout.fillWidth: true
                Layout.preferredHeight: preferredIconSize
                Layout.preferredWidth: preferredIconSize
                visible: !isPlaceHolder

                source: isPlaceHolder ? "" : modelData.icon

                LingmoCore.ToolTipArea {
                    id: iconToolTip
                    anchors.fill: parent
                    mainText: {
                        if (!modelData.condition) {
                            return "";
                        }
                        if (!modelData.probability) {
                            return modelData.condition;
                        }
                        return i18nc("certain weather condition (probability percentage)",
                                     "%1 (%2%)", modelData.condition, modelData.probability);
                    }
                }
            }

            LingmoComponents.Label {
                // Position it closer to the weather condition icon
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                Layout.topMargin: -LingmoUI.Units.smallSpacing
                Layout.bottomMargin: LingmoUI.Units.smallSpacing * 2

                horizontalAlignment: Text.AlignHCenter
                // i18n: \ufe0e forces the text representation of the umbrella emoji
                text: modelData.probability ? i18nc("Probability of precipitation in percentage", "\ufe0e☂%1%", modelData.probability) : "·"
                textFormat: Text.PlainText
                visible: root.rowHasProbability[index % root.rows] && !!modelData.icon
            }

            LingmoComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: !isPlaceHolder && modelData.tempHigh || i18nc("Short for no data available", "-")
                textFormat: Text.PlainText
                visible: !isPlaceHolder && (modelData.tempHigh || !showNightRow)
            }

            LingmoComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: !isPlaceHolder && modelData.tempLow || i18nc("Short for no data available", "-")
                textFormat: Text.PlainText
                visible: !isPlaceHolder && (modelData.tempLow || !showNightRow)
            }
        }
    }
}
