/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Window
import QtQuick3D
import org.kde.kwin as KWinComponents

import "constants.js" as Constants

Item {
    id: root
    focus: true

    readonly property QtObject targetScreen: KWinComponents.SceneView.screen

    function switchTo(desktop) {
        KWinComponents.Workspace.currentDesktop = desktop;
        effect.deactivate();
    }

    function switchToSelected() {
        const eulerRotation = cameraController.rotation.toEulerAngles();
        switchTo(cube.desktopAt(eulerRotation.y));
    }

    View3D {
        id: view
        anchors.fill: parent
        renderMode: View3D.Underlay

        Loader {
            id: transparentSceneEnvironment
            active: effect.configuration.Background == Constants.Background.Color
            sourceComponent: SceneEnvironment {
                backgroundMode: SceneEnvironment.Transparent

                // When using View3D.Underlay, SceneEnvironment.clearColor will do nothing.
                Binding {
                    target: root.Window.window
                    property: "color"
                    value: effect.configuration.BackgroundColor
                }
            }
        }

        Loader {
            id: skyboxSceneEnvironment
            active: effect.configuration.Background == Constants.Background.SkyBox
            sourceComponent: SceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBox
                lightProbe: Texture {
                    source: effect.configuration.SkyBox
                }
            }
        }

        environment: {
            if (skyboxSceneEnvironment.active) {
                return skyboxSceneEnvironment.item;
            } else {
                return transparentSceneEnvironment.item;
            }
        }

        PerspectiveCamera {
            id: camera
            clipNear: 10.0
            clipFar: 100000.0
        }

        Cube {
            id: cube
            faceDisplacement: effect.configuration.CubeFaceDisplacement
            faceSize: Qt.size(root.width, root.height)
        }

        CubeCameraController {
            id: cameraController
            anchors.fill: parent
            state: effect.activated ? "distant" : "close"
            camera: camera
            xInvert: effect.configuration.MouseInvertedX
            yInvert: effect.configuration.MouseInvertedY

            states: [
                State {
                    name: "close"
                    PropertyChanges {
                        target: cameraController
                        radius: cube.faceDistance + 0.5 * cube.faceSize.height / Math.tan(0.5 * camera.fieldOfView * Math.PI / 180)
                        rotation: Quaternion.fromEulerAngles(0, cube.desktopAzimuth(KWinComponents.Workspace.currentDesktop), 0)
                    }
                },
                State {
                    name: "distant"
                    PropertyChanges {
                        target: cameraController
                        radius: cube.faceDistance * effect.configuration.DistanceFactor + 0.5 * cube.faceSize.height / Math.tan(0.5 * camera.fieldOfView * Math.PI / 180)
                        rotation: Quaternion.fromEulerAngles(0, cube.desktopAzimuth(KWinComponents.Workspace.currentDesktop), 0).times(Quaternion.fromEulerAngles(-20, 0, 0))
                    }
                }
            ]

            Behavior on rotation {
                enabled: !cameraController.busy
                QuaternionAnimation {
                    id: rotationAnimation
                    duration: effect.animationDuration
                    easing.type: Easing.OutCubic
                }
            }
            Behavior on radius {
                NumberAnimation {
                    duration: effect.animationDuration
                    easing.type: Easing.OutCubic
                }
            }

            function rotateToLeft() {
                if (rotationAnimation.running) {
                    return;
                }
                const eulerAngles = rotation.toEulerAngles();
                let next = Math.floor(eulerAngles.y / cube.angleTick) * cube.angleTick;
                if (Math.abs(next - eulerAngles.y) < 0.05 * cube.angleTick) {
                    next -= cube.angleTick;
                }
                rotation = Quaternion.fromEulerAngles(0, next - eulerAngles.y, 0).times(rotation);
            }

            function rotateToRight() {
                if (rotationAnimation.running) {
                    return;
                }
                const eulerAngles = rotation.toEulerAngles();
                let next = Math.ceil(eulerAngles.y / cube.angleTick) * cube.angleTick;
                if (Math.abs(next - eulerAngles.y) < 0.05 * cube.angleTick) {
                    next += cube.angleTick;
                }
                rotation = Quaternion.fromEulerAngles(0, next - eulerAngles.y, 0).times(rotation);
            }
        }
    }

    MouseArea {
        anchors.fill: view
        onClicked: mouse => {
            const hitResult = view.pick(mouse.x, mouse.y);
            if (hitResult.objectHit) {
                root.switchTo(hitResult.objectHit.desktop);
            } else {
                root.switchToSelected();
            }
        }
    }

    Keys.onEscapePressed: effect.deactivate();
    Keys.onLeftPressed: cameraController.rotateToLeft();
    Keys.onRightPressed: cameraController.rotateToRight();
    Keys.onEnterPressed: root.switchToSelected();
    Keys.onReturnPressed: root.switchToSelected();
    Keys.onSpacePressed: root.switchToSelected();
}
