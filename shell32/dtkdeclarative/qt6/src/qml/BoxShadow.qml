// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0 as D

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
    property bool hollow: false
    readonly property real __offsetX: hollow ? shadowOffsetX : 0
    readonly property real __offsetY: hollow ? shadowOffsetY : 0
    readonly property real __spread: hollow ? spread : 0
    readonly property real __borderBase: cornerRadius + spread + shadowBlur * 2
    readonly property real __minImageSize: 2 * __borderBase
    readonly property real __boxSize: __minImageSize - 2 * shadowBlur - 2 * __spread + 1

    D.DBorderImage {
        id: image

        anchors {
            centerIn: parent
            horizontalCenterOffset:  shadowOffsetX
            verticalCenterOffset: shadowOffsetY
        }
        width: parent.width + (shadowBlur + spread) * 2
        height: parent.height + (shadowBlur + spread) * 2

        source: D.DTK.makeShadowImageUrl(__boxSize, shadowRadius(topLeftRadius), shadowRadius(topRightRadius), shadowRadius(bottomLeftRadius), shadowRadius(bottomRightRadius),
                                         shadowBlur, shadowColor, __offsetX, __offsetY, __spread, hollow, false)
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
            left: Math.min(width / 2, __borderBase - __offsetX)
            right: Math.min(width / 2, __borderBase + __offsetX)
            top: Math.min(height / 2, __borderBase - __offsetY)
            bottom: Math.min(height / 2, __borderBase + __offsetY)
        }
    }
}
