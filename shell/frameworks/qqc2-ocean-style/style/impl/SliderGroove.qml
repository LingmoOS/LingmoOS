/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later OR LicenseRef-KDE-Accepted-LGPL OR LicenseRef-KFQF-Accepted-GPL
 */

import QtQuick
import QtQuick.Templates as Templates
import org.kde.lingmoui as LingmoUI

import "." as Impl

Rectangle {
    id: root

    property Templates.Control control
    property real startPosition: isRangeSlider ? control.first.position : 0
    property real endPosition: isRangeSlider ? control.second.position : control.position

    readonly property bool isRangeSlider: control instanceof Templates.RangeSlider

    readonly property real handleWidth: isRangeSlider ? control.first.handle.width ?? 0 : control.handle.width ?? 0
    readonly property real handleHeight: isRangeSlider ? control.first.handle.height ?? 0 : control.handle.height ?? 0
    readonly property real secondHandleWidth: isRangeSlider ? control.second.handle.width ?? 0 : handleWidth
    readonly property real secondHandleHeight: isRangeSlider ? control.second.handle.height ?? 0 : handleHeight

    readonly property bool horizontal: root.control.horizontal
    readonly property bool vertical: root.control.vertical


    //NOTE: Manually setting x,y,width,height because that's what the Basic, Fusion and Material QQC2 styles do.
    // Inset would be more idiomatic for QQC2, but this is easier to deal with for now since the behavior is expected by app devs.

    x: control.leftPadding + (root.horizontal ?
        (control.mirrored ? root.secondHandleWidth/2 : root.handleWidth/2) - radius
        : (control.availableWidth - width) / 2)
    y: control.topPadding + (root.vertical ? root.secondHandleHeight/2 - radius : (control.availableHeight - height) / 2)

    implicitWidth: root.horizontal ? 200 : Impl.Units.grooveHeight
    implicitHeight: root.vertical ? 200 : Impl.Units.grooveHeight

    width: root.horizontal ? control.availableWidth - root.handleWidth/2 - secondHandleWidth/2 + Impl.Units.grooveHeight : implicitWidth
    height: root.vertical ? control.availableHeight - root.handleHeight/2 - secondHandleHeight/2 + Impl.Units.grooveHeight : implicitHeight

    radius: Impl.Units.grooveHeight/2
    color: LingmoUI.Theme.backgroundColor
    border {
        width: Impl.Units.smallBorder
        color: Impl.Theme.separatorColor()
    }

    Rectangle {
        id: fill
        anchors {
            fill: parent
            leftMargin: root.horizontal ? root.startPosition * parent.width - (root.startPosition * Impl.Units.grooveHeight) : 0
            rightMargin: root.horizontal ? (1-root.endPosition) * parent.width - ((1-root.endPosition) * Impl.Units.grooveHeight) : 0
            topMargin: root.vertical ? (1-root.endPosition) * parent.height - ((1-root.endPosition) * Impl.Units.grooveHeight) : 0
            bottomMargin: root.vertical ? root.startPosition * parent.height - (root.startPosition * Impl.Units.grooveHeight) : 0
        }

        radius: parent.radius
        color: LingmoUI.Theme.alternateBackgroundColor
        border {
            width: Impl.Units.smallBorder
            color: LingmoUI.Theme.focusColor
        }

        Behavior on anchors.leftMargin {
            enabled: fill.loaded && !LingmoUI.Settings.hasTransientTouchInput
            SmoothedAnimation {
                duration: LingmoUI.Units.longDuration
                velocity: 800
                //SmoothedAnimations have a hardcoded InOutQuad easing
            }
        }
        Behavior on anchors.rightMargin {
            enabled: fill.loaded && !LingmoUI.Settings.hasTransientTouchInput
            SmoothedAnimation {
                duration: LingmoUI.Units.longDuration
                velocity: 800
            }
        }
        Behavior on anchors.topMargin {
            enabled: fill.loaded && !LingmoUI.Settings.hasTransientTouchInput
            SmoothedAnimation {
                duration: LingmoUI.Units.longDuration
                velocity: 800
            }
        }
        Behavior on anchors.bottomMargin {
            enabled: fill.loaded && !LingmoUI.Settings.hasTransientTouchInput
            SmoothedAnimation {
                duration: LingmoUI.Units.longDuration
                velocity: 800
            }
        }

        // Prevents animations from running when loaded
        // HACK: for some reason, this won't work without a 1ms timer
        property bool loaded: false
        Timer {
            id: awfulHackTimer
            interval: 1
            onTriggered: fill.loaded = true
        }
        Component.onCompleted: {
            awfulHackTimer.start()
        }
    }

    //NOTE: this code has problems when large from/to ranges are used.
    // Keeping it here to work on it later.
    /*
    Loader {
        id: tickmarkLoader
        visible: root.control.stepSize > 0
        active: visible
        anchors {
            left: root.horizontal ? parent.left : parent.right
            top: root.vertical ? parent.top : parent.bottom
            leftMargin: root.horizontal ? parent.radius : Impl.Units.smallBorder
            topMargin: root.vertical ? parent.radius : Impl.Units.smallBorder
        }
        width: root.vertical ? implicitWidth : root.width - parent.radius
        height: root.horizontal ? implicitHeight : root.height - parent.radius
        sourceComponent: markGridComponent
    }

    Loader {
        id: tickmarkLoader2
        visible: tickmarkLoader.visible
        active: visible
        anchors {
            left: parent.left
            top: parent.top
            leftMargin: root.horizontal ? parent.radius : -width - Impl.Units.smallBorder
            topMargin: root.vertical ? parent.radius : -height - Impl.Units.smallBorder
        }
        width: tickmarkLoader.width
        height: tickmarkLoader.height
        sourceComponent: markGridComponent
    }

    Component {
        id: markGridComponent
        Grid {
            id: markGrid
            rows: root.vertical ? markRepeater.model : 1
            columns: root.horizontal ? markRepeater.model : 1
            spacing: (root.vertical ? height/(markRepeater.model-1) : width/(markRepeater.model-1)) - Impl.Units.smallBorder*2
            Repeater {
                id: markRepeater
                model: (root.control.to - root.control.from)/root.control.stepSize + 1
                delegate: Rectangle {
                    implicitWidth: root.vertical ? root.x - Impl.Units.smallBorder : Impl.Units.smallBorder
                    implicitHeight: root.horizontal ? root.y - Impl.Units.smallBorder : Impl.Units.smallBorder
                    color: (root.horizontal && x >= fill.x && x <= fill.x + fill.width)
                        || (root.vertical && y >= fill.y && y <= fill.y + fill.height)
                        ? LingmoUI.Theme.focusColor
                        : Impl.Theme.separatorColor()
                }
            }
        }
    }*/
}
