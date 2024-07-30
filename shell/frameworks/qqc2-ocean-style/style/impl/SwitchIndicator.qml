/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import "." as Impl

Item {
    id: root

    property T.AbstractButton control: root.parent
    property bool mirrored: control.mirrored
    readonly property bool controlHasContent: control.contentItem && control.contentItem.width > 0

    implicitWidth: implicitHeight*2
    implicitHeight: Impl.Units.inlineControlHeight

    x: controlHasContent ? (root.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
    y: control.topPadding + (control.availableHeight - height) / 2

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false

    Rectangle {
        id: background
        anchors {
            fill: parent
            margins: Math.floor(parent.height / 6)
        }
        radius: height / 2
        color: LingmoUI.Theme.backgroundColor
        border {
            width: Impl.Units.smallBorder
            color: Impl.Theme.separatorColor()
        }
    }

    Rectangle {
        id: fillEffectRect
        visible: width > handle.width/2
        color: LingmoUI.Theme.alternateBackgroundColor
        border {
            width: Impl.Units.smallBorder
            color: LingmoUI.Theme.focusColor
        }
        radius: height/2
        anchors {
            left: background.left
            right: handle.horizontalCenter
            top: background.top
            bottom: background.bottom
        }
    }

    /* For some reason, if I try to turn the handle into a reusable component
     * like a SliderHandle, the fillEffectRect can't anchor to the handle:
     * "Cannot anchor to an item that isn't a parent or sibling."
     * Except, it is a sibling. Even if I set `parent: root` on the fillEffectRect
     * and the handle, I get the error. (╯°□°）╯︵ ┻━┻
     */
    Rectangle {
        id: handle
        anchors {
            top: parent.top
            bottom: parent.bottom
        }
        // It's necessary to use x position instead of anchors so that the handle position can be dragged
        x: Math.max(
            0,
            Math.min(
                parent.width - width,
                control.visualPosition * parent.width - (width / 2)
            )
        )
        width: height
        radius: height / 2
        color: LingmoUI.Theme.backgroundColor
        border {
            width: Impl.Units.smallBorder
            color: control.down || control.visualFocus || control.hovered ?
                LingmoUI.Theme.focusColor : Impl.Theme.separatorColor()
        }

        Behavior on border.color {
            enabled: control.down || control.visualFocus || control.hovered
            ColorAnimation {
                duration: LingmoUI.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }

        Behavior on x {
            enabled: handle.loaded// && !LingmoUI.Settings.hasTransientTouchInput
            // Using SmoothedAnimation because the fill effect is anchored to the handle.
            SmoothedAnimation {
                duration: LingmoUI.Units.shortDuration
                //SmoothedAnimations have a hardcoded InOutQuad easing
            }
        }

        SmallBoxShadow {
            id: shadow
            opacity: control.down ? 0 : 1
            visible: control.enabled
            radius: parent.radius
        }

        FocusRect {
            baseRadius: handle.radius
            visible: control.visualFocus
        }

        // Prevents animations from running when loaded
        property bool loaded: false
        Component.onCompleted: {
            loaded = true
        }
    }
}
