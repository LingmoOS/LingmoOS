/*
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.StackView {
    id: control

    // Using NumberAnimation instead of XAnimator because the latter wasn't always smooth enough.
    pushEnter: Transition {
        NumberAnimation {
            property: "x"
            from: (control.mirrored ? -0.5 : 0.5) * control.width
            to: 0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 0.0; to: 1.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    pushExit: Transition {
        NumberAnimation {
            property: "x"
            from: 0
            to: (control.mirrored ? -0.5 : 0.5) * -control.width
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 1.0; to: 0.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    popEnter: Transition {
        NumberAnimation {
            property: "x"
            from: (control.mirrored ? -0.5 : 0.5) * -control.width
            to: 0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 0.0; to: 1.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    popExit: Transition {
        NumberAnimation {
            property: "x"
            from: 0
            to: (control.mirrored ? -0.5 : 0.5) * control.width
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 1.0; to: 0.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    replaceEnter: Transition {
        NumberAnimation {
            property: "x"
            from: (control.mirrored ? -0.5 : 0.5) * control.width
            to: 0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 0.0; to: 1.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    replaceExit: Transition {
        NumberAnimation {
            property: "x"
            from: 0
            to: (control.mirrored ? -0.5 : 0.5) * -control.width
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 1.0; to: 0.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
}
