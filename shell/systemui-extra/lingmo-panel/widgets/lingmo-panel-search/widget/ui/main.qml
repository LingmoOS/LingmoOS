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

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.lingmo.quick.widgets 1.0
import org.lingmo.panel.search 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? childrenRect.width : width
    Layout.preferredHeight: isHorizontal ? height : childrenRect.height

    StyleBackground {
        id: backGround
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: root.isHorizontal? 4 : 0
        anchors.leftMargin: root.isHorizontal? 0 : 4
        width: root.isHorizontal? icon.width + searchText.width + 32 : parent.width - 8
        height: root.isHorizontal? parent.height - 8 : icon.height + 16
        radius: root.isHorizontal? height / 2 : width / 2
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: mouseArea.containsPress? 0.15 : mouseArea.containsMouse? 0.05 : 0
        border.width: 1
        borderColor: Theme.BrightText
        borderAlpha: mouseArea.containsPress? 0.15 : 0.05
        ThemeIcon {
            id: icon
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 8
            width: root.isHorizontal? height : parent.width - 16
            height: root.isHorizontal? parent.height - 16 : width
            source: "lingmo-search"
            mode: Icon.AutoHighlight
        }
        StyleText {
            id: searchText
            anchors.left: icon.right
            anchors.top: backGround.top
            anchors.leftMargin: 8
            anchors.topMargin: (parent.height - height) / 2
            height: contentHeight
            width: contentWidth
            text: qsTr("Search")
            visible: root.isHorizontal
            alpha: 0.55
        }
    }
    StyleBackground {
        anchors.fill: backGround
        z: -1
        paletteRole: Theme.Base
        useStyleTransparency: false
        alpha: 0.95
        radius: backGround.radius
    }
    MouseArea {
        id: mouseArea
        anchors.fill: backGround
        hoverEnabled: true

        SearchButton {
            id: searchButton
        }

        onClicked: {
            searchButton.LingmoUISearch();
        }
        Tooltip {
            id: tooltip
            anchors.fill: parent
            mainText: Widget.tooltip
            posFollowCursor: false
            location: {
                switch(Widget.container.position) {
                    case Types.Bottom:
                        return Dialog.BottomEdge;
                    case Types.Left:
                        return Dialog.LeftEdge;
                    case Types.Top:
                        return Dialog.TopEdge;
                    case Types.Right:
                        return Dialog.RightEdge;
                }
            }
            margin: 6
        }
    }

    Widget.actions:[
        Action {
            iconName: "document-page-setup-symbolic"
            text: qsTr("Search settings")
            onTriggered: function () {
                searchButton.openSearchSetting();
            }
        }
    ]


}
