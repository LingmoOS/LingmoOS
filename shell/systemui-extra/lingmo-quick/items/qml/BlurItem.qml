/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
import QtQuick 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id: root
    property variant source
    property int samples: 9
    property real blurRadius: Math.floor(samples / 2)
    property bool hideSource: false
    color: "transparent"

    layer.enabled: true
    layer.effect: OpacityMask {
        maskSource: Rectangle {
            width: root.width
            height: root.height
            radius: root.radius
        }
    }

    ShaderEffectSource {
        id: effectSource
        sourceItem: root.source
        sourceRect: Qt.rect(root.x, root.y, root.width, root.height)
        hideSource: root.hideSource
        width: parent.width
        height: parent.height
    }

    GaussianBlur {
        anchors.fill: parent
        source: effectSource
        radius: root.blurRadius
        samples: root.samples
    }
    //TODO:
    //关闭特效时，仅绘制固定透明度的base色
}
