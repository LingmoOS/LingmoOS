/*
 *   SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>
 *   SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PlasmaComponents

Item {

    property int value: 30

    KSvg.SvgItem {
        id: face
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width / (naturalSize.width / naturalSize.height)
        imagePath: "widgets/analog_meter"
        elementId: "background"
    }

    KSvg.SvgItem {
        id: centerScrew
        imagePath: "widgets/analog_meter"
        elementId: "rotatecenter"
        rotation: value + 90

        x: face.x + meterSvg.elementRect("rotatecenter").x * svgScale
        y: face.y + meterSvg.elementRect("rotatecenter").y * svgScale

        property real svgScale: face.width / face.naturalSize.width
        width: naturalSize.width * svgScale
        height: naturalSize.height * svgScale

        KSvg.SvgItem {
            imagePath: "widgets/analog_meter"

            anchors.horizontalCenter: parent.horizontalCenter
            y: x
            elementId: "pointer-shadow"
            width: naturalSize.width * centerScrew.svgScale
            height: naturalSize.height * centerScrew.svgScale
        }
        KSvg.SvgItem {
            id: hand
            imagePath: "widgets/analog_meter"
            elementId: "pointer"
            anchors.horizontalCenter: parent.horizontalCenter
            y: x

            width: naturalSize.width * centerScrew.svgScale
            height: naturalSize.height * centerScrew.svgScale
        }
    }
    KSvg.SvgItem {
        imagePath: "widgets/analog_meter"
        elementId: "foreground"
        anchors.centerIn: centerScrew

        width: naturalSize.width * centerScrew.svgScale
        height: naturalSize.height * centerScrew.svgScale
    }
    PlasmaComponents.Label {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: face.bottom
        }
        text: model.imagePath
        visible: width < parent.width
    }

}
