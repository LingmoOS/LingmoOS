// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import Waylib.Server
import TreeLand.Protocols

OutputItem {
    id: rootOutputItem
    required property WaylandOutput waylandOutput
    property OutputViewport onscreenViewport: outputViewport
    property Cursor waylandCursor

    output: waylandOutput
    devicePixelRatio: waylandOutput.scale
    cursorDelegate: Item {
        required property OutputCursor cursor

        visible: cursor.visible

        Image {
            id: cursorImage
            visible: !cursor.isHardwareCursor
            source: cursor.imageSource
            x: -cursor.hotspot.x
            y: -cursor.hotspot.y
            cache: false
            width: cursor.size.width
            height: cursor.size.height
            sourceClipRect: cursor.sourceRect
        }

        SurfaceItem {
            id: dragIcon
            z: cursorImage.z - 1
            flags: SurfaceItem.DontCacheLastBuffer
            visible: waylandCursor.dragSurface !== null
            surface: waylandCursor.dragSurface
        }
    }

    OutputViewport {
        id: outputViewport

        output: waylandOutput
        devicePixelRatio: parent.devicePixelRatio
        anchors.centerIn: parent

        RotationAnimation {
            id: rotationAnimator

            target: outputViewport
            duration: 200
            alwaysRunToEnd: true
        }

        Timer {
            id: setTransform

            property var scheduleTransform
            onTriggered: onscreenViewport.rotateOutput(scheduleTransform)
            interval: rotationAnimator.duration / 2
        }

        function rotationOutput(orientation) {
            setTransform.scheduleTransform = orientation
            setTransform.start()

            switch(orientation) {
            case WaylandOutput.R90:
                rotationAnimator.to = 90
                break
            case WaylandOutput.R180:
                rotationAnimator.to = 180
                break
            case WaylandOutput.R270:
                rotationAnimator.to = -90
                break
            default:
                rotationAnimator.to = 0
                break
            }

            rotationAnimator.from = rotation
            rotationAnimator.start()
        }
    }

    Image {
        id: background

        fillMode: Image.PreserveAspectCrop
        anchors.fill: parent
        asynchronous: true

        Component.onCompleted: {
            let name = PersonalizationV1.getOutputName(waylandOutput);
            background.source = PersonalizationV1.background(name) + "?" + new Date().getTime()
            WallpaperColorV1.updateWallpaperColor(name, PersonalizationV1.backgroundIsDark(name));
        }

        Connections {
            target: PersonalizationV1
            function onBackgroundChanged(outputName, isdark) {
                let name = PersonalizationV1.getOutputName(waylandOutput);
                if (outputName === name) {
                    background.source = PersonalizationV1.background(outputName) + "?" + new Date().getTime()
                    WallpaperColorV1.updateWallpaperColor(outputName, isdark);
                }
            }
        }
    }

    Component {
        id: outputScaleEffect

        OutputViewport {
            readonly property OutputItem outputItem: waylandOutput.OutputItem.item

            id: viewport
            input: this
            output: waylandOutput
            devicePixelRatio: outputViewport.devicePixelRatio

            TextureProxy {
                sourceItem: outputViewport
                anchors.fill: parent
            }

            Item {
                width: outputItem.width
                height: outputItem.height
                anchors.centerIn: parent
                rotation: -outputViewport.rotation

                Item {
                    y: 10
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width / 2
                    height: parent.height / 3
                    clip: true

                    Item {
                        id: centerItem
                        width: 1
                        height: 1
                        anchors.centerIn: parent
                        rotation: outputViewport.rotation

                        TextureProxy {
                            id: magnifyingLens

                            sourceItem: outputViewport
                            smooth: false
                            scale: 10
                            transformOrigin: Item.TopLeft
                            width: viewport.width
                            height: viewport.height

                            function updatePosition() {
                                const pos = outputItem.lastActiveCursorItem.mapToItem(outputViewport, Qt.point(0, 0))
                                x = - pos.x * scale
                                y = - pos.y * scale
                            }

                            Connections {
                                target: outputItem.lastActiveCursorItem

                                function onXChanged() {
                                    magnifyingLens.updatePosition()
                                }

                                function onYChanged() {
                                    magnifyingLens.updatePosition()
                                }
                            }
                            Component.onCompleted: updatePosition()
                        }
                    }
                }
            }
        }
    }

    function setTransform(transform) {
        onscreenViewport.rotationOutput(transform)
    }

    function setScale(scale) {
        onscreenViewport.setOutputScale(scale)
    }

    Component.onDestruction: onscreenViewport.invalidate()
}
