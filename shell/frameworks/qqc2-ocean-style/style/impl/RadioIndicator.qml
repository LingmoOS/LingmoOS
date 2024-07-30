/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import "." as Impl

// TODO: replace with ShadowedRectangle because it produces smoother circles.
// Unfortunately I can't do it now because batching issues can cause all radio
// buttons to show hover effects when only one is hovered
Rectangle {
    id: root

    property T.AbstractButton control: root.parent
    property bool mirrored: control.mirrored
    readonly property bool controlHasContent: control.contentItem && control.contentItem.width > 0

    implicitWidth: implicitHeight
    implicitHeight: Impl.Units.inlineControlHeight

    x: controlHasContent ? (root.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
    y: control.topPadding + (control.availableHeight - height) / 2

    radius: width / 2

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false
    color: control.down || control.checked ? LingmoUI.Theme.alternateBackgroundColor : LingmoUI.Theme.backgroundColor

    border {
        width: Impl.Units.smallBorder
        color: control.down || control.checked || control.visualFocus || control.hovered ? LingmoUI.Theme.focusColor : Impl.Theme.separatorColor();
    }

    Behavior on color {
        enabled: control.down || control.checked
        ColorAnimation {
            duration: LingmoUI.Units.shortDuration
            easing.type: Easing.OutCubic
        }
    }

    Behavior on border.color {
        enabled: control.down || control.checked || control.visualFocus || control.hovered
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

    Rectangle {
        id: mark
        anchors.centerIn: parent
        implicitHeight: {
            let h = root.height * 0.4
            h -= h % 2
            // we need an odd width because it's centered, so there needs to be a center pixel
            return Math.round(h + 1)
        }
        implicitWidth: implicitHeight
        radius: height / 2
        color: LingmoUI.Theme.textColor
        visible: control.checked
        scale: 0.8
    }

    FocusRect {
        baseRadius: root.radius
        visible: control.visualFocus
    }

    states: [
        State {
            when: !control.checked
            name: "unchecked"
            PropertyChanges {
                target: mark
                scale: 0.8
            }
        },
        State {
            when: control.checked
            name: "checked"
            PropertyChanges {
                target: mark
                scale: 1
            }
        }
    ]

    transitions: [
        /* Using `from: "unchecked"` instead of `from: "*"` prevents the transition
         * from running when the parent control is created.
         * This can reduce resource usage spikes on pages that have way too many
         * controls with indicators.
         */
        Transition {
            from: "unchecked"
            to: "checked"
            ScaleAnimator {
                duration: LingmoUI.Units.shortDuration
                easing.type: Easing.OutQuad
            }
        }
    ]
}
