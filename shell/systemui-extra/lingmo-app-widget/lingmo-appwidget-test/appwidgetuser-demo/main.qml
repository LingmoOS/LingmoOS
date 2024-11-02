/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12

ApplicationWindow {
    id: root
    visible: true
    width: screen.width
    height: screen.height
    title: qsTr("qmlWindow")
    property int intervalx: 40
    property int intervaly: 20

    Rectangle {
        id: rec1
        width: parent.width
        height: (2*(parent.height))/3
        anchors.margins: 10
        color: "teal"
        border.color: "black"
        ListView {
            id: listview1
            orientation: ListView.Horizontal
            width: rec1.width-2*intervalx
            height: rec1.height-2*intervaly
            anchors.verticalCenter: rec1.verticalCenter
            model: AppWidgetModel
            spacing: 20

            delegate: Rectangle {
                id: rectangle1
                width: 500
                height: 500
                color:"transparent"
                Loader {
                    id: loader
                    anchors.fill: rectangle1
                    asynchronous: false
                    source: "file://" + appQml
                    onLoaded: {
                        item.register(item.appname, "user");
                    }
                }
            }
        }
    }
    Rectangle {
        id: rec2
        y: (2*(parent.height))/3
        width: parent.width
        height: (parent.height)/3
        anchors.margins: 10
        border.color: "black"
        Button {
            id: but2
            x: intervalx
            y: 2*intervaly
            text: "Iconlist"
            onClicked: pop.open()
        }
        Popup {
            id: pop
            width: rec2.width-2*intervalx
            height: rec2.height-2*intervaly
            x: intervalx
            y: intervaly
            ListView {
                orientation: ListView.Horizontal
                id: listview
                width: rec2.width-2*intervalx
                height: rec2.height-2*intervaly
                model: KListModel
                spacing: 20

                delegate: Rectangle {
                    id: wraper
                    width: 200
                    height: 200

                    MouseArea {
                        id: mouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            parent.color = "blue"
                        }
                        onExited: {
                            parent.color = "lightblue"
                        }
                        onClicked: {
                            console.log("onclicked" + text1.text)
                            console.log("onclicked" + itemName)
                            AppWidgetModel.insert(text1.text,text1.text)
                        }
                    }
                    Text {
                        id: text1
                        anchors.bottom: wraper.bottom
                        anchors.horizontalCenter: wraper.horizontalCenter
                        text: itemName
                        Layout.preferredWidth: 120
                        color: "black"
                        font.pointSize: 15
                    }
                    Image {
                        id: image1
                        width: 100
                        height:100
                        anchors.centerIn: wraper
                        source: "file://" + itemIcon
                    }
                }
            }
        }
    }
}
