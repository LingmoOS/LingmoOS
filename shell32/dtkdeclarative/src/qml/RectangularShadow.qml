// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk.impl 1.0 as D

Item {
    id: rootItem

    /*! This property defines the offset of the shadow in the x-axis direction. */
    property real offsetX: 0.0

    /*! This property defines the offset of the shadow in the y-axis direction. */
    property real offsetY: 0.0

    /*! This property defines how many pixels outside(or inside) the item area are reached by the shadow. */
    property real glowRadius: 0.0

    /*! This property defines how large part of the shadow color is strengthened near the source edges. */
    property real spread: 0.0

    /*! This property defines the the shadow color. */
    property color color: "black"

    /*! This property defines corners size of the control that draws the shadow. */
    property real cornerRadius: glowRadius

    /*! This property defines does the shadow fill the entire area. */
    property alias fill: effct.fill

    D.GlowEffect {
        id: effct

        property real inverseSpread: 1.0 - rootItem.spread
        property real cornerRadius: {
            var maxCornerRadius = Math.min(rootItem.width, rootItem.height) / 2 + rootItem.glowRadius;
            return Math.max(0, Math.min(rootItem.cornerRadius + rootItem.glowRadius, maxCornerRadius))
        }

        x: (parent.width - width) / 2.0 + offsetX
        y: (parent.height - height) / 2.0 + offsetY
        width: parent.width + rootItem.glowRadius * 2 + cornerRadius * 2
        height: parent.height + rootItem.glowRadius * 2 + cornerRadius * 2
        color: rootItem.color
        spread: rootItem.spread / 2.0
        glowRadius: rootItem.glowRadius * 1 + cornerRadius * 2
        relativeSizeX: ((inverseSpread * inverseSpread) * rootItem.glowRadius + cornerRadius * 2.0) / width
        relativeSizeY: relativeSizeX * (width / height)
    }
}
