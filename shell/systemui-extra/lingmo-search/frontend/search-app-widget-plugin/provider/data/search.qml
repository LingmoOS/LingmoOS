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
 */
import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import org.lingmo.appwidget 1.0
//import org.lingmo.qqc2style.private 1.0 as StylePrivate

AppWidget {
    visible: true
    id: window
    appname: "search"
    username: parent.username
    anchors.fill: parent

    property real scaling: parent.width / 480

    function parseJson(jsonStr) {
        try {
            var obj = JSON.parse(jsonStr);
            return obj;
        } catch(e) {
            console.log("json解析失败");
        }
        return undefined;
    }

    function updateSearchBarColor() {
//        for (var i in window.datavalue) {
//            if (i === "colorData") {
//                var data = window.parseJson(window.datavalue[i]);
//                searchBar.color = Qt.rgba(data.red, data.green, data.blue, data.alpha);
//                placeholderText.color = data.placeHolderTextColor;
//            }
//        }

        var data = window.parseJson(window.datavalue["colorData"]);
        searchBar.color = Qt.rgba(data.red, data.green, data.blue, data.alpha);
        placeholderText.color = data.placeHolderTextColor;
    }

    onDatavalueChanged: updateSearchBarColor();

    onUserstatusChanged: {
        if (userstatus === "Normal") {
            mouseArea.enabled = true;
        } else if (userstatus === "Editable") {
            mouseArea.enabled = false;
        }
    }

    Rectangle {
        id:searchBar
        anchors.centerIn: parent
        width : 448 * scaling
        height: 64 * scaling
        radius: searchBar.height / 2

        MouseArea {
            id: mouseArea
            anchors.fill: parent;
            propagateComposedEvents: true;

            onClicked: {
                window.qmldefineeventchangedsignal("clicked", "search", "");
            }
            onPressAndHold: {
                mouse.accepted = false;
            }
        }

        RowLayout {
            id: layout
            anchors.fill: parent;
            spacing: 0;

            Image {
                id: image_search
                Layout.leftMargin: 16 * scaling;
                Layout.rightMargin: 16 * scaling;
                Layout.preferredWidth: 32 * scaling;
                Layout.preferredHeight: 32 * scaling;

                Layout.alignment: Qt.AlignVCenter;
                source: "file:///usr/share/appwidget/search/lingmo-search.svg";
            }

            Item {
                clip: true;
                Layout.fillWidth: true;
                Layout.fillHeight: true;
                Layout.rightMargin: 32 * scaling;

                Label {
                    id: searchtext
                    anchors.fill: parent;

//                    Keys.enabled: true;
//                    Keys.onEnterPressed: {
//                        window.qmldefineeventchangedsignal("enter", "search", searchtext.text);
//                        searchtext.remove(0, searchtext.length);
//                    }
//                    Keys.onReturnPressed: {
//                        window.qmldefineeventchangedsignal("return", "search", searchtext.text);
//                        searchtext.remove(0, searchtext.length);
//                    }

//                    focus: true;
//                    color: StylePrivate.StyleHelper.windowtextcolorrole;
//                    maximumLength: 100 * scaling;
//                    selectByMouse: true;
//                    verticalAlignment: Qt.AlignVCenter;
//                    font.pixelSize: 21 * scaling;

                    property string placeholderText: qsTr("search")/*"全局搜索"*/

                    Text {
                        id: placeholderText
                        text: searchtext.placeholderText
                        font.pixelSize: 21 * scaling

                        visible: !searchtext.text;
                        anchors.verticalCenter: parent.verticalCenter;
                    }
                }
            }
        }
    }
}


