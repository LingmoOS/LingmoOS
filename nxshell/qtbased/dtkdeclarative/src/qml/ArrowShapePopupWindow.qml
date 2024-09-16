// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Window {
    id: __window

    property alias roundJoinRadius: arrowShape.roundJoinRadius
    property alias arrowWidth: arrowShape.arrowWidth
    property alias arrowHeight: arrowShape.arrowHeight
    property alias arrowX: arrowShape.arrowX
    property alias arrowY: arrowShape.arrowY
    property alias arrowDirection: arrowShape.arrowDirection

    flags: Qt.Popup
    D.DWindow.enabled: true
    D.DWindow.clipPath: loader.item
    D.DWindow.enableBlurWindow: true
    D.DWindow.alphaBufferSize: D.DTK.hasBlurWindow ? 8 : 0

    ArrowShapeContainer {
        id: arrowShape
        anchors.fill: parent
        color: "transparent"
    }
}
