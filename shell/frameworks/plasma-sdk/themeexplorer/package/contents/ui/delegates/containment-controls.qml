/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.20 as Kirigami

Item {
    Rectangle {
        id: background
        anchors {
            fill: parent
            margins: Kirigami.Units.gridUnit
        }
        radius: 3
        color: Kirigami.Theme.backgroundColor
        opacity: 0.6
    }
    KSvg.FrameSvgItem {
        anchors {
            left: background.left
            right: background.right
            verticalCenter: background.verticalCenter
        }
        height: maxSlider.height + minSlider.height
        imagePath: "widgets/containment-controls"
        prefix: "south"
        KSvg.SvgItem {
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }
            imagePath: "widgets/containment-controls"
            elementId: "south-offsetslider"
        }
        KSvg.SvgItem {
            id: maxSlider
            anchors {
                top: parent.top
                right: parent.right
            }
            imagePath: "widgets/containment-controls"
            elementId: "south-maxslider"
        }
        KSvg.SvgItem {
            id: minSlider
            anchors {
                bottom: parent.bottom
                right: parent.right
            }
            imagePath: "widgets/containment-controls"
            elementId: "south-minslider"
        }
        KSvg.SvgItem {
            anchors {
                top: parent.top
                left: parent.left
            }
            imagePath: "widgets/containment-controls"
            elementId: "south-minslider"
        }
        KSvg.SvgItem {
            anchors {
                bottom: parent.bottom
                left: parent.left
            }
            imagePath: "widgets/containment-controls"
            elementId: "south-maxslider"
        }
    }

    PlasmaComponents.Label {
        anchors {
            horizontalCenter: background.horizontalCenter
            bottom: background.bottom
        }
        text: model.imagePath
        visible: width < background.width
    }
}
