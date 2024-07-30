/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmoui 2 as LingmoUI

Rectangle {
    id: root
    color: "black"

    property int stage

    onStageChanged: {
        if (stage == 2) {
            introAnimation.running = true;
        } else if (stage == 5) {
            introAnimation.target = busyIndicator;
            introAnimation.from = 1;
            introAnimation.to = 0;
            introAnimation.running = true;
        }
    }

    Item {
        id: content
        anchors.fill: parent
        opacity: 0

        Image {
            id: logo
            //match SDDM/lockscreen avatar positioning
            readonly property real size: LingmoUI.Units.gridUnit * 8

            anchors.centerIn: parent

            asynchronous: true
            source: "images/lingmo.svgz"

            sourceSize.width: size
            sourceSize.height: size
        }

        // TODO: port to LingmoComponents3.BusyIndicator
        Image {
            id: busyIndicator
            //in the middle of the remaining space
            y: parent.height - (parent.height - logo.y) / 2 - height/2
            anchors.horizontalCenter: parent.horizontalCenter
            asynchronous: true
            source: "images/busywidget.svgz"
            sourceSize.height: LingmoUI.Units.gridUnit * 2
            sourceSize.width: LingmoUI.Units.gridUnit * 2
            RotationAnimator on rotation {
                id: rotationAnimator
                from: 0
                to: 360
                // Not using a standard duration value because we don't want the
                // animation to spin faster or slower based on the user's animation
                // scaling preferences; it doesn't make sense in this context
                duration: 2000
                loops: Animation.Infinite
                // Don't want it to animate at all if the user has disabled animations
                running: LingmoUI.Units.longDuration > 1
            }
        }
        Row {
            spacing: LingmoUI.Units.largeSpacing
            anchors {
                bottom: parent.bottom
                right: parent.right
                margins: LingmoUI.Units.gridUnit
            }
            Text {
                color: "#eff0f1"
                anchors.verticalCenter: parent.verticalCenter
                text: i18ndc("lingmo_lookandfeel_org.kde.lookandfeel", "This is the first text the user sees while starting in the splash screen, should be translated as something short, is a form that can be seen on a product. Lingmo is the project name so shouldn't be translated.", "Lingmo made by KDE")
            }
            Image {
                asynchronous: true
                source: "images/kde.svgz"
                sourceSize.height: LingmoUI.Units.gridUnit * 2
                sourceSize.width: LingmoUI.Units.gridUnit * 2
            }
        }
    }

    OpacityAnimator {
        id: introAnimation
        running: false
        target: content
        from: 0
        to: 1
        duration: LingmoUI.Units.veryLongDuration * 2
        easing.type: Easing.InOutQuad
    }
}
