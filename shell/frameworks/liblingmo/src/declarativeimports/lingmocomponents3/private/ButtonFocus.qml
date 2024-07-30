/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.ksvg as KSvg
import org.kde.lingmoui as LingmoUI

KSvg.FrameSvgItem {
    anchors {
        fill: parent
        leftMargin: -margins.left
        topMargin: -margins.top
        rightMargin: -margins.right
        bottomMargin: -margins.bottom
    }

    property bool showFocus: false
    property bool flat: false

    imagePath: "widgets/button"
    prefix: flat ? ["toolbutton-focus", "focus"] : "focus"

    opacity: showFocus ? 1 : 0
    Behavior on opacity {
        enabled: LingmoUI.Units.shortDuration > 0
        OpacityAnimator { duration: LingmoUI.Units.shortDuration; easing.type: Easing.OutQuad }
    }
}
