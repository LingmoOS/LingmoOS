/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import org.kde.ksvg as KSvg
//NOTE: importing LingmoCore is necessary in order to make KSvg load the current Lingmo Theme
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI
import "private" as P

Item {
    id: root

    required property T.AbstractButton control

    property size hintSize: radioButtonSvg.fromCurrentImageSet && radioButtonSvg.hasElement("hint-size")
        ? radioButtonSvg.elementSize("hint-size")
        : Qt.size(LingmoUI.Units.iconSizes.small, LingmoUI.Units.iconSizes.small)

    implicitWidth: hintSize.width
    implicitHeight: hintSize.height
    opacity: control.enabled ? 1 : 0.5
    layer.enabled: opacity < 1

    KSvg.Svg {
        id: radioButtonSvg
        imagePath: "widgets/radiobutton"
    }

    Loader {
        anchors.fill: parent
        sourceComponent: radioButtonSvg.fromCurrentImageSet
            // Hardcode ocean-light and ocean-dark because fromCurrentImageSet is
            // false for them. This is because they don't contain any SVGs and
            // inherit all of them from the default theme.
            || KSvg.ImageSet.imageSetName === "ocean-light"
            || KSvg.ImageSet.imageSetName === "ocean-dark"
            ? radiobuttonComponent : compatibilityComponent
    }

    // Uses newer radiobutton.svg
    Component {
        id: radiobuttonComponent
        KSvg.SvgItem {
            svg: radioButtonSvg
            elementId: "normal"
            anchors.centerIn: parent
            implicitWidth: naturalSize.width
            implicitHeight: naturalSize.height
            KSvg.SvgItem {
                z: -1
                svg: radioButtonSvg
                elementId: "shadow"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: opacity > 0
                opacity: enabled && !root.control.down
                Behavior on opacity {
                    enabled: root.control.down && LingmoUI.Units.longDuration > 0
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
            KSvg.SvgItem {
                svg: radioButtonSvg
                elementId: "checked"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: opacity > 0
                opacity: (root.control.checked || root.control.down)
                    && !(root.control instanceof T.ItemDelegate && root.control.highlighted)
                Behavior on opacity {
                    enabled: (root.control.checked || root.control.down) && LingmoUI.Units.longDuration > 0
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
            KSvg.SvgItem {
                svg: radioButtonSvg
                elementId: "focus"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: opacity > 0
                opacity: root.control.visualFocus
                Behavior on opacity {
                    enabled: LingmoUI.Units.longDuration > 0
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
            KSvg.SvgItem {
                svg: radioButtonSvg
                elementId: "hover"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: opacity > 0
                opacity: root.control.hovered
                Behavior on opacity {
                    enabled: root.control.hovered && LingmoUI.Units.longDuration > 0
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
            KSvg.SvgItem {
                svg: radioButtonSvg
                elementId: "symbol"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: scale > 0
                scale: root.control.checked
                Behavior on scale {
                    enabled: LingmoUI.Units.longDuration > 0
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }
    }

    // Uses older combination of actionbutton.svg and checkmarks.svg.
    // NOTE: Do not touch this except to fix bugs. This is for compatibility.
    Component {
        id: compatibilityComponent
        KSvg.SvgItem {
            svg: KSvg.Svg {
                id: buttonSvg
                imagePath: "widgets/actionbutton"
            }
            elementId: "normal"

            anchors.centerIn: parent
            implicitWidth: implicitHeight
            implicitHeight: LingmoUI.Units.iconSizes.small

            KSvg.SvgItem {
                id: checkmark
                svg: KSvg.Svg {
                    id: checkmarksSvg
                    imagePath: "widgets/checkmarks"
                }
                elementId: "radiobutton"
                opacity: root.control.checked ? 1 : 0
                anchors {
                    fill: parent
                }
                Behavior on opacity {
                    enabled: LingmoUI.Units.longDuration > 0
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }
            P.RoundShadow {
                anchors.fill: parent
                z: -1
                state: root.control.activeFocus ? "focus" : (root.control.hovered ? "hover" : "shadow")
            }
        }
    }
}
