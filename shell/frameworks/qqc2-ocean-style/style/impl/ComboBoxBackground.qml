/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

/*
 * This file exists mainly because ComboBox's `highlighted` has the same name,
 * but a completely different meaning from Button's `highlighted`.
 *
 * ComboBox::highlighted(int index)
 * This signal is emitted when the item at index in the popup list is highlighted by the user.
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import "." as Impl

Rectangle {
    id: mainBackground

    property T.ComboBox control: mainBackground.parent

    property color flatColor: Qt.rgba(
        LingmoUI.Theme.backgroundColor.r,
        LingmoUI.Theme.backgroundColor.g,
        LingmoUI.Theme.backgroundColor.b,
        0
    )
    property bool highlightBackground: control.down
    property bool highlightBorder: control.down || control.visualFocus || control.hovered

    implicitWidth: 200
    implicitHeight: Impl.Units.mediumControlHeight

    visible: !control.flat || control.editable || control.down || control.visualFocus || control.hovered

    color: {
        if (highlightBackground) {
            return LingmoUI.Theme.alternateBackgroundColor
        } else if (control.flat) {
            return flatColor
        } else {
            return LingmoUI.Theme.backgroundColor
        }
    }

    border {
        color: highlightBorder ?
            LingmoUI.Theme.focusColor : Impl.Theme.buttonSeparatorColor()
        width: Impl.Units.smallBorder
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

    radius: Impl.Units.smallRadius

    SmallBoxShadow {
        id: shadow
        opacity: control.down ? 0 : 1
        visible: !control.editable && !control.flat && control.enabled
        radius: parent.radius
    }

    FocusRect {
        id: focusRect
        baseRadius: mainBackground.radius
        visible: control.visualFocus
    }
}
