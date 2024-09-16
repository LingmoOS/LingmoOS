// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk.impl 1.0 as D

Item {
    id: root

    property real cornerRadius: 0
    property variant topLeftRadius
    property variant topRightRadius
    property variant bottomLeftRadius
    property variant bottomRightRadius
    property real shadowBlur: 10
    property real shadowOffsetX: 0
    property real shadowOffsetY: 0
    property color shadowColor: "black"
    property real spread: 0
    readonly property real __borderBase: cornerRadius + spread + shadowBlur / 2.0
    readonly property real __minImageSize: Math.max(image.border.left + image.border.right, image.border.top + image.border.bottom)
    readonly property real __boxSize: Math.max(__minImageSize - shadowBlur, cornerRadius * 2 + 1)

    BorderImage {
        id: image

        anchors.fill: parent
        source: D.DTK.makeShadowImageUrl(__boxSize, shadowRadius(topLeftRadius), shadowRadius(topRightRadius), shadowRadius(bottomLeftRadius), shadowRadius(bottomRightRadius),
                                         shadowBlur, shadowColor, shadowOffsetX, shadowOffsetY, spread, false, true)
        function shadowRadius(radius) {
            return radius !== undefined ? radius : cornerRadius
        }

        function bound(min, val, max) {
            if (val <= min)
                return min;
            if (val >= max)
                return max;
            return val;
        }

        border {
            left: __borderBase + bound(0, shadowBlur + shadowOffsetX, width - 2 * __borderBase)
            right: __borderBase + bound(0, shadowBlur - shadowOffsetX, width - 2 * __borderBase)
            top: __borderBase + bound(0, shadowBlur + shadowOffsetY, height - 2 * __borderBase)
            bottom: __borderBase + bound(0, shadowBlur - shadowOffsetY, height - 2 * __borderBase)
        }
    }
}
