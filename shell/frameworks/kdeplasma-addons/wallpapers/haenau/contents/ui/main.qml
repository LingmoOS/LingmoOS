/*
 *   SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.plasmoid 2.0

WallpaperItem {
    id: wallpaper

    //FIXME: Svg doesn't support relative paths
    readonly property string wallpaperPath: String(Qt.resolvedUrl("wallpaper.svgz")).substring(7)

    KSvg.SvgItem {
        anchors.fill: parent

        elementId: "background"

        BackgroundElement {
            id: centerLayer0
            elementId: "center-layer0"
        }
        BackgroundElement {
            id: centerLayer1
            elementId: "center-layer1"
        }

        BackgroundElement {
            id: leftLayer0
            elementId: "left-layer0"
        }

        BackgroundElement {
            id: leftLayer1
            elementId: "left-layer1"

            BackgroundElement {
                id: leftLayer2
                elementId: "left-layer2"
            }
            BackgroundElement {
                id: leftLayer3
                elementId: "left-layer3"
            }
            BackgroundElement {
                id: leftLayer4
                elementId: "left-layer4"
            }
        }

        BottomBackgroundElement {
            id: bottomLayer0
            elementId: "bottom-layer0"

            BottomBackgroundElement {
                id: bottomLayer1
                elementId: "bottom-layer1"
            }
            BottomBackgroundElement {
                id: bottomLayer2
                elementId: "bottom-layer2"
            }
        }

        RightBackgroundElement {
            id: rightLayer0
            elementId: "right-layer0"
        }

        RightBackgroundElement {
            id: rightLayer1
            elementId: "right-layer1"

            RightBackgroundElement {
                id: rightLayer2
                elementId: "right-layer2"
            }
            RightBackgroundElement {
                id: rightLayer3
                elementId: "right-layer3"
            }
            RightBackgroundElement {
                id: rightLayer4
                elementId: "right-layer4"
            }
        }

        Timer {
            property int lastLayer: 0
            property variant layers: [centerLayer0, centerLayer1, leftLayer0, leftLayer1, bottomLayer0, rightLayer0, rightLayer1]
            running: true
            repeat: true
            interval: 30000
            onTriggered: {
                layers[lastLayer].change()
                lastLayer = (lastLayer + 1) % 7
            }
        }

    }
}
