// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk.style 1.0 as DS

T.StackView {
    id: control

    popEnter: Transition {
        // slide_in_left
        NumberAnimation { property: "x"; from: (control.mirrored ? -0.5 : 0.5) *  -control.width; to: 0; duration: DS.Style.stackView.animationDuration; easing.type: DS.Style.stackView.animationEasingType }
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: DS.Style.stackView.animationDuration; easing.type: DS.Style.stackView.animationEasingType }
    }

    popExit: Transition {
        // slide_out_right
        NumberAnimation { property: "x"; from: 0; to: (control.mirrored ? -0.5 : 0.5) * control.width; duration: DS.Style.stackView.animationDuration; easing.type: Easing.OutCubic }
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: DS.Style.stackView.animationDuration; easing.type: DS.Style.stackView.animationEasingType }
    }

    pushEnter: Transition {
        // slide_in_right
        NumberAnimation { property: "x"; from: (control.mirrored ? -0.5 : 0.5) * control.width; to: 0; duration: DS.Style.stackView.animationDuration; easing.type: Easing.OutCubic }
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: DS.Style.stackView.animationDuration; easing.type: DS.Style.stackView.animationEasingType }
    }

    pushExit: Transition {
        // slide_out_left
        NumberAnimation { property: "x"; from: 0; to: (control.mirrored ? -0.5 : 0.5) * -control.width; duration: DS.Style.stackView.animationDuration; easing.type: Easing.OutCubic }
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: DS.Style.stackView.animationDuration; easing.type: DS.Style.stackView.animationEasingType }
    }

    replaceEnter: Transition {
        // slide_in_right
        NumberAnimation { property: "x"; from: (control.mirrored ? -0.5 : 0.5) * control.width; to: 0; duration: DS.Style.stackView.animationDuration; easing.type: Easing.OutCubic }
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: DS.Style.stackView.animationDuration; easing.type: DS.Style.stackView.animationEasingType }
    }

    replaceExit: Transition {
        // slide_out_left
        NumberAnimation { property: "x"; from: 0; to: (control.mirrored ? -0.5 : 0.5) * -control.width; duration: DS.Style.stackView.animationDuration; easing.type: Easing.OutCubic }
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: DS.Style.stackView.animationDuration; easing.type: DS.Style.stackView.animationEasingType }
    }
}
