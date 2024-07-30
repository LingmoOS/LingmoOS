/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import "." as Impl

Rectangle {
    id: root

    property T.Control control: root.parent
    property real position: control.position
    property real visualPosition: control.visualPosition
    property bool hovered: control.hovered
    property bool pressed: control.pressed
    property bool visualFocus: control.visualFocus

    property bool usePreciseHandle: false

    implicitWidth: implicitHeight
    implicitHeight: Impl.Units.inlineControlHeight

    // It's not necessary here. Not sure if it would swap leftPadding with
    // rightPadding in the x position calculation, but there's no risk to
    // being safe here.
    LayoutMirroring.enabled: false

    // It's necessary to use x and y positions instead of anchors so that the handle position can be dragged
    x: {
        let xPos = 0
        if (control.horizontal) {
            xPos = root.visualPosition * (control.availableWidth - width)
        } else {
            xPos = (control.availableWidth - width) / 2
        }
        return xPos + control.leftPadding
    }
    y: {
        let yPos = 0
        if (control.vertical) {
            yPos = root.visualPosition * (control.availableHeight - height)
        } else {
            yPos = (control.availableHeight - height) / 2
        }
        return yPos + control.topPadding
    }

    rotation: root.vertical && usePreciseHandle ? -90 : 0

    radius: height / 2
    color: LingmoUI.Theme.backgroundColor
    border {
        width: Impl.Units.smallBorder
        color: root.pressed || root.visualFocus || root.hovered ? LingmoUI.Theme.focusColor : Impl.Theme.separatorColor()
    }

    Behavior on border.color {
        enabled: root.pressed || root.visualFocus || root.hovered
        ColorAnimation {
            duration: LingmoUI.Units.shortDuration
            easing.type: Easing.OutCubic
        }
    }

    Behavior on x {
        enabled: root.loaded && !LingmoUI.Settings.hasTransientTouchInput
        SmoothedAnimation {
            duration: LingmoUI.Units.longDuration
            velocity: 800
            //SmoothedAnimations have a hardcoded InOutQuad easing
        }
    }
    Behavior on y {
        enabled: root.loaded && !LingmoUI.Settings.hasTransientTouchInput
        SmoothedAnimation {
            duration: LingmoUI.Units.longDuration
            velocity: 800
        }
    }

    SmallBoxShadow {
        id: shadow
        opacity: root.pressed ? 0 : 1
        visible: control.enabled
        radius: parent.radius
    }

    FocusRect {
        baseRadius: root.radius
        visible: root.visualFocus
    }

    // Prevents animations from running when loaded
    // HACK: for some reason, this won't work without a 1ms timer
    property bool loaded: false
    Timer {
        id: awfulHackTimer
        interval: 1
        onTriggered: root.loaded = true
    }
    Component.onCompleted: {
        awfulHackTimer.start()
    }
}
