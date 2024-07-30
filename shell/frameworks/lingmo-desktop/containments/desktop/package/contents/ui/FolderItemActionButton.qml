/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.ksvg 1.0 as KSvg

KSvg.SvgItem {
    id: actionButton

    width: {
        if (!visible) {
            return 0;
        }
        switch (Plasmoid.configuration.iconSize) {
            case 0: return LingmoUI.Units.iconSizes.small;
            case 1: return LingmoUI.Units.iconSizes.small;
            case 2: return LingmoUI.Units.iconSizes.smallMedium;
            case 3: return LingmoUI.Units.iconSizes.smallMedium;
            case 4: return LingmoUI.Units.iconSizes.smallMedium;
            case 5: return LingmoUI.Units.iconSizes.medium;
            case 6: return LingmoUI.Units.iconSizes.large;
            default: return LingmoUI.Units.iconSizes.small;
        }
    }
    height: width

    signal clicked()

    property string element

    svg: KSvg.Svg {
        imagePath: "widgets/action-overlays"
        multipleImages: true
        size: "16x16"
    }
    elementId: element + "-normal"

    Behavior on opacity {
        NumberAnimation { duration: LingmoUI.Units.shortDuration }
    }

    MouseArea {
        id: actionButtonMouseArea

        anchors.fill: actionButton

        acceptedButtons: Qt.LeftButton
        hoverEnabled: true

        onClicked: mouse => actionButton.clicked()

        states: [
            State {
                name: "hover"
                when: actionButtonMouseArea.containsMouse && !actionButtonMouseArea.pressed

                PropertyChanges {
                    target: actionButton
                    elementId: actionButton.element + "-hover"
                }
            },
            State {
                name: "pressed"
                when: actionButtonMouseArea.pressed

                PropertyChanges {
                    target: actionButton
                    elementId: actionButton.element + "-pressed"
                }
            }
        ]
    }
}
