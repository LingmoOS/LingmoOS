/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents

GridLayout {
    property var generalModel
    property var observationModel

    readonly property int sideWidth: Math.max(
        windSpeedLabel.implicitWidth,
        tempLabel.implicitWidth,
        windSpeedDirection.naturalSize.width
    )

    Layout.minimumWidth: Math.max(
        locationLabel.implicitWidth,
        (sideWidth + columnSpacing) * 2 + Kirigami.Units.iconSizes.huge /* conditionIcon.Layout.minimumWidth */
    )

    visible: !!generalModel.location

    columnSpacing: Kirigami.Units.gridUnit
    rowSpacing: Kirigami.Units.gridUnit

    columns: 3

    Kirigami.Heading {
        id: locationLabel

        Layout.row: 0
        Layout.column: 0
        Layout.columnSpan: 3
        Layout.fillWidth: true

        wrapMode: Text.NoWrap

        text: generalModel.location
        textFormat: Text.PlainText
    }

    PlasmaComponents.Label {
        id: tempLabel

        Layout.row: 1
        Layout.column: 0
        Layout.fillWidth: true
        Layout.preferredWidth: 25 // 25% of the view
        Layout.minimumWidth: sideWidth
        Layout.alignment: Qt.AlignCenter

        font.pixelSize: Kirigami.Units.iconSizes.medium
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.NoWrap

        text: observationModel.temperature
        textFormat: Text.PlainText
    }

    Kirigami.Icon {
        id: conditionIcon

        Layout.row: 1
        Layout.column: 1
        Layout.minimumHeight: Kirigami.Units.iconSizes.huge
        Layout.minimumWidth: Kirigami.Units.iconSizes.huge
        Layout.preferredHeight: Layout.minimumHeight
        Layout.fillWidth: true
        // All the items have `fillWidth: true`, so the layout weights each
        // contribution and splits the space accordingly to their proportion.
        Layout.preferredWidth: 50 // 50% of the view

        source: generalModel.currentConditionIconName
    }

    PlasmaComponents.Label {
        id: conditionLabel

        Layout.row: 2
        Layout.column: 0
        Layout.columnSpan: 3
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

        text: observationModel.conditions
        textFormat: Text.PlainText
    }

    Item {
        Layout.row: 1
        Layout.column: 2
        Layout.fillWidth: true
        Layout.preferredWidth: 25 // 25% of the view
        Layout.minimumWidth: sideWidth
        Layout.alignment: Qt.AlignCenter

        implicitHeight: windSpeedDirection.implicitHeight + windSpeedLabel.implicitHeight

        KSvg.SvgItem {
            id: windSpeedDirection

            anchors.horizontalCenter: parent.horizontalCenter
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium

            imagePath: "weather/wind-arrows"
            elementId: observationModel.windDirectionId || ""

            visible: !!observationModel.windDirectionId
        }

        PlasmaComponents.Label {
            id: windSpeedLabel
            anchors {
                top: windSpeedDirection.bottom
                horizontalCenter: parent.horizontalCenter
            }
            text: observationModel.windSpeed
            textFormat: Text.PlainText
        }
    }

}
