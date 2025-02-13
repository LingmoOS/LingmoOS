/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import Lingmo.FileManager 1.0

Item {
    id: _listItem
    width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
    height: ListView.view.itemHeight

    Accessible.name: fileName
    Accessible.role: Accessible.Canvas

    property Item iconArea: _image.visible ? _image : _icon
    property Item labelArea: _label
    property Item labelArea2: _label2

    property int index: model.index
    property bool hovered: ListView.view.hoveredItem === _listItem
    property bool selected: model.selected
    property bool blank: model.blank

    property color hoveredColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.backgroundColor, 2.3)
                                                       : Qt.darker(LingmoUI.Theme.backgroundColor, 1.05)
    property color selectedColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.backgroundColor, 1.2)
                                                        : Qt.darker(LingmoUI.Theme.backgroundColor, 1.15)
//    onSelectedChanged: {
//        if (selected && !blank) {
//            _listItem.grabToImage(function(result) {
//                folderModel.addItemDragImage(_listItem.index,
//                                             _listItem.x,
//                                             _listItem.y,
//                                             _listItem.width, _listItem.height, result.image)
//            })
//        }
//    }

    Rectangle {
        id: _background
        anchors.fill: parent
        radius: LingmoUI.Theme.smallRadius
        color: selected ? LingmoUI.Theme.highlightColor : hovered ? hoveredColor : "transparent"
        visible: selected || hovered
        opacity: selected ? 0.1 : 2
    }

    RowLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.leftMargin: LingmoUI.Units.smallSpacing
        anchors.rightMargin: LingmoUI.Units.smallSpacing
        spacing: LingmoUI.Units.largeSpacing

        Item {
            id: iconItem
            Layout.fillHeight: true
            width: parent.height * 0.8
            opacity: model.isHidden ? 0.5 : 1.0

            Image {
                id: _icon
                anchors.centerIn: iconItem
                width: iconItem.width
                height: width
                sourceSize.width: width
                sourceSize.height: height
                source: "image://icontheme/" + model.iconName
                visible: !_image.visible
                asynchronous: true
            }

            Image {
                id: _image
                width: parent.height * 0.8
                height: width
                anchors.centerIn: iconItem
                sourceSize: Qt.size(_icon.width, _icon.height)
                source: model.thumbnail ? model.thumbnail : ""
                visible: _image.status === Image.Ready
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                cache: false

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: _image.width
                        height: _image.height

                        Rectangle {
                            anchors.centerIn: parent
                            width: Math.min(parent.width, _image.paintedWidth)
                            height: Math.min(parent.height, _image.paintedHeight)
                            radius: height * 0.1
                        }
                    }
                }
            }

            Image {
                anchors.right: _icon.visible ? _icon.right : _image.right
                anchors.bottom: _icon.visible ? _icon.bottom : _image.bottom
                source: "image://icontheme/emblem-symbolic-link"
                width: 16
                height: 16
                visible: model.isLink
                sourceSize: Qt.size(width, height)
            }
        }

        ColumnLayout {
            spacing: 0

            Label {
                id: _label
                text: model.fileName
                Layout.fillWidth: true
                color: selected ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.textColor
                textFormat: Text.PlainText
                elide: Qt.ElideMiddle
                opacity: model.isHidden ? 0.8 : 1.0
            }

            Label {
                id: _label2
                text: model.fileSize
                color: selected ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.disabledTextColor
                textFormat: Text.PlainText
                Layout.fillWidth: true
                opacity: model.isHidden ? 0.8 : 1.0
            }
        }

        Label {
            text: model.modified
            textFormat: Text.PlainText
            color: selected ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.disabledTextColor
            opacity: model.isHidden ? 0.8 : 1.0
        }
    }
}
