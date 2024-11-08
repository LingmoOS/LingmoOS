/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
import QtQuick 2.12
import QtQuick.Layouts 1.15
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

StyleBackground {
    id: root
    property Item tooltip
    paletteRole: Theme.Base
    paletteGroup: Theme.Active
    radius: Theme.normalRadius
    borderColor: Theme.Shadow
    border.width: 1
    borderAlpha: 0.15
    useStyleTransparency: false
    width: toolTipText.contentWidth + 20
    height: toolTipText.contentHeight + 20
    Layout.minimumWidth: childrenRect.width + Theme.fontSize
    Layout.minimumHeight: childrenRect.height + Theme.fontSize

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    StyleText {
        id: toolTipText
        anchors.centerIn: parent
        text: tooltip ? tooltip.mainText : ""
        textFormat: tooltip ? tooltip.isRichText ? Text.RichText : Text.AutoText : Text.AutoText
    }
}
