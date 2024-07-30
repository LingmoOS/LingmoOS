/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean

import "." as Impl

Rectangle {
    id: root

    property T.AbstractButton control: root.parent
    property int checkState: control.checkState
    property int symbolSize: Impl.Units.symbolSize(Math.min(width, height))
    property bool mirrored: control.mirrored
    readonly property bool controlHasContent: control.contentItem && control.contentItem.width > 0

    property bool highlightBackground: root.checkState !== Qt.Unchecked || (control.down && !(control instanceof T.CheckDelegate))
    property bool highlightBorder: control.down || root.checkState !== Qt.Unchecked || control.highlighted || control.visualFocus || control.hovered

    visible: control.checkable

    x: controlHasContent ? (root.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
    y: control.topPadding + (control.availableHeight - height) / 2

    implicitWidth: implicitHeight
    implicitHeight: Impl.Units.inlineControlHeight

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false
    color: highlightBackground ? LingmoUI.Theme.alternateBackgroundColor : LingmoUI.Theme.backgroundColor

    radius: Impl.Units.smallRadius

    border {
        width: Impl.Units.smallBorder
        color: highlightBorder ?
            LingmoUI.Theme.focusColor : Impl.Theme.separatorColor()
            //LingmoUI.ColorUtils.tintWithAlpha(root.color, LingmoUI.Theme.textColor, 0.3)
    }

    Behavior on color {
        enabled: highlightBackground
        ColorAnimation {
            duration: LingmoUI.Units.shortDuration
            easing.type: Easing.OutCubic
        }
    }
    Behavior on border.color {
        enabled: highlightBorder
        ColorAnimation {
            duration: LingmoUI.Units.shortDuration
            easing.type: Easing.OutCubic
        }
    }

    SmallBoxShadow {
        id: shadow
        opacity: control.down ? 0 : 1
        visible: control.enabled
        radius: parent.radius
    }

    PaintedSymbol {
        id: checkmark
        anchors.centerIn: parent
        // Should reliably create pixel aligned checkmarks that don't get cut off on the sides.
        height: root.symbolSize + penWidth*2
        width: height
        color: LingmoUI.Theme.textColor
        symbolType: PaintedSymbol.Checkmark
        visible: root.checkState === Qt.Checked

        /* RTL support. Horizontally flips the checkmark.
         * Is this actually a good idea for checkmarks?
        transform: control.mirrored ? horizontalFlipMatrix : null

        Matrix4x4 {
            id: horizontalFlipMatrix
            matrix: Qt.matrix4x4(
                -1, 0, 0, checkmark.width,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            )
        }
        */
    }

    Item {
        id: partialCheckmark
        visible: root.checkState === Qt.PartiallyChecked
        anchors.centerIn: parent
        width: root.symbolSize
        height: 2

        Rectangle {
            id: leftRect
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height
            width: height
            color: LingmoUI.Theme.textColor
        }

        Rectangle {
            id: middleRect
            anchors.centerIn: parent
            height: parent.height
            width: height
            color: LingmoUI.Theme.textColor
        }

        Rectangle {
            id: rightRect
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height
            width: height
            color: LingmoUI.Theme.textColor
        }
    }

    FocusRect {
        baseRadius: root.radius
        visible: control.visualFocus
    }

    Rectangle {
        id: sidewaysRevealRect
        antialiasing: true
        anchors {
            right: checkmark.right
            top: checkmark.top
            bottom: checkmark.bottom
        }
        width: 0
        visible: width > 0
        color: root.color
    }

    NumberAnimation {
        id: sidewaysRevealAnimation
        target: sidewaysRevealRect
        property: "width"
        from: checkmark.width
        to: 0
        duration: LingmoUI.Units.shortDuration
        //Intentionally not using an easing curve
    }

    states: [
        State {
            name: "unchecked"
            when: root.checkState === Qt.Unchecked
        },
        State {
            name: "checked"
            when: root.checkState === Qt.Checked
        },
        State {
            name: "partiallychecked"
            when: root.checkState === Qt.PartiallyChecked
        }
    ]

    property bool loaded: false
    Component.onCompleted: {
        loaded = true
    }

    /* I'm using this instead of transitions because I couldn't reliably
     * trigger the animation when going from the partiallychecked state to the
     * checked state.
     */
    onStateChanged: {
        /* Prevents the transition from running when the parent control is created.
         * This can reduce resource usage spikes on pages that have way too many checkboxes.
         */
        if (root.loaded
            && (state == "checked" || state == "partiallychecked")
            && !LingmoUI.Settings.isMobile //TODO: make the animation look better on mobile
        ) {
            // equivalent to stop(), then start()
            sidewaysRevealAnimation.restart()
        }
    }
}
