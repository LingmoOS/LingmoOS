/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.formatter as Formatter

GridLayout {
    id: grid

    property bool compact

    readonly property real preferredWidth: titleMetrics.width

    readonly property int rowCount: Math.ceil(gridRepeater.count / columns)

    columnSpacing: compact ? 1 : Kirigami.Units.largeSpacing
    rowSpacing: compact ? 1 : Kirigami.Units.largeSpacing

    Kirigami.Heading {
        id: heading
        Layout.fillWidth: true
        Layout.columnSpan: parent.columns > 0 ? parent.columns : 1

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        text: root.controller.title
        visible: !grid.compact && root.controller.showTitle && text.length > 0
        level: 2

        TextMetrics {
            id: titleMetrics
            font: heading.font
            text: heading.text
        }
    }

    Repeater {
        id: gridRepeater

        model: root.controller.highPrioritySensorIds

        FaceControl {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 0
            Layout.preferredHeight: 0
            compact: grid.compact
            controller: root.controller
            sensors: [modelData]
            faceId: root.controller.faceConfiguration.faceId
        }
    }
}
