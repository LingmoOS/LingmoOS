/*
    SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Templates 2.12

ApplicationWindow {
    width: control.implicitWidth + gridUnit*4
    height: control.implicitHeight + gridUnit*4
    color: "white"

    property int gridUnit: fontMetrics.height

    FontMetrics {
        id: fontMetrics
    }

    Button {
        id: control

        implicitWidth: Math.round(Math.max(implicitContentWidth + leftPadding + rightPadding))
        implicitHeight: Math.round(Math.max(implicitContentHeight + topPadding + bottomPadding))

        anchors.centerIn: parent

        padding: 8

        // This is the default behavior, I'm just making it explicit.
        hoverEnabled: Qt.styleHints.useHoverEffects

        text: "There should be a red outline hover effect on this button"

        contentItem: Label {
            text: control.text
            font: control.font
            color: "white"
        }

        background: Rectangle {
            radius: 3
            color: "black"
            border.color: "red"
            // When the useHoverEffects style hint is true, the hover effect should work
            border.width: control.hovered ? 4 : 0
        }
    }
}
