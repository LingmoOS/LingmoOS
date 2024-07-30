/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

import "." as Impl

LingmoUI.ShadowedRectangle {
    id: root
    property int currentIndex: -1
    property int count: 0
    property bool alwaysCurveCorners: false
    readonly property real topRadius: (alwaysCurveCorners || root.currentIndex == 0) ? Impl.Units.smallRadius : 0
    readonly property real bottomRadius: (alwaysCurveCorners || root.currentIndex == Math.max(root.count-1, 0)) ? Impl.Units.smallRadius : 0

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Selection
    LingmoUI.Theme.inherit: false

    corners {
        topLeftRadius: root.topRadius
        topRightRadius: root.topRadius
        bottomLeftRadius: root.bottomRadius
        bottomRightRadius: root.bottomRadius
    }

    color: LingmoUI.Theme.alternateBackgroundColor
    border {
        width: Impl.Units.smallBorder
        color: LingmoUI.Theme.focusColor
    }
}
