// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQuick.Shapes 1.10
import org.deepin.dtk 1.0

Column {
    id: control

    Row {
        Button {
            text: "popup window"
            onClicked: {
    //            popup.x = (Screen.desktopAvailableWidth - Window.window.width) / 2
    //            popup.y = (Screen.desktopAvailableHeight - Window.window.height) / 2

    //            popupWindow.width = 400
                popupWindow.open()
    //            popupWindow.PopupWindow.window.hide()
    //            Window.window.hide()
    //            popup.PopupWindow.window.show()
    //            popupWindow.PopupWindow.close()
            }
    //        Component.onCompleted: popupWindow.open()
        }
        Button {
            text: "change x"
            onClicked: {
                popupWindow.x = 500
                popupWindow.open()
            }
        }
        Button {
            text: "change width"
            onClicked: {
                popupWindow.width = 400
                popupWindow.open()
            }
        }

        Button {
            text: "popup control"
            onClicked: {
    //            popupWindow.x = (Screen.desktopAvailableWidth - Window.window.width) / 2
    //            popupWindow.y = (Screen.desktopAvailableHeight - Window.window.height) / 2

                popup.open()
            }
        }
    }

    Row {
        Button {
            text: "menu control"
            onClicked: {
                menu.open()
            }
        }

        Button {
            text: "menu popupWindow"
            onClicked: {
                menuPopup.open()
            }
        }
    }
    Button {
        text: "handle forceWindowMode"
        onClicked: {
            popupWindow.PopupHandle.forceWindowMode = !popupWindow.PopupHandle.forceWindowMode
        }
    }

    Popup {
        id: popupWindow; objectName: "pupup window"
        visible: false
        x: 100
        y: 20
//        width: 200
//        height: 100
//        width: 300
//        height: 300
//        margins: 100
        PopupHandle.forceWindowMode: true
        PopupHandle.delegate: PopupWindow {
            blurControl: popupWindow
        }
        contentItem: Column {
            spacing: 10
            Text {
                text: "蓝牙"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Button {
                text: "打印机"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
    Popup {
        id: popup;  objectName: "pupup"
        x: 100
        y: 20
//        margins: 100
        contentItem: Column {
            spacing: 10
            Text {
                text: "蓝牙"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Button {
                text: "打印机"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
    Menu {
        id: menu
        MenuItem { text: "Text" }
    }
    Menu {
        id: menuPopup
        MenuItem { text: "Text" }

        PopupHandle.forceWindowMode: true
    }
    ArrowShapePopup {
        id: arrow
        x: 50
        y: 50
        width: 100
        height: 100
    }

    Row {
        Button {
            text: "Up"
            onClicked: {
                arrow.arrowDirection = ArrowBoxPath.Up
                arrow.arrowX = arrow.width / 2 + 10
                arrow.arrowY = 10
                arrow.open()
            }
        }
        Button {
            text: "Down"
            onClicked: {
                arrow.arrowDirection = ArrowBoxPath.Down
                arrow.arrowX = arrow.width / 2 + 10
                arrow.arrowY = arrow.height - 10
                arrow.open()
            }
        }
        Button {
            text: "Left"
            onClicked: {
                arrow.arrowDirection = ArrowBoxPath.Left
                arrow.arrowX = 10
                arrow.arrowY = arrow.height / 2 + 10
                arrow.open()
            }
        }
        Button {
            text: "Right"
            onClicked: {
                arrow.arrowDirection = ArrowBoxPath.Right
                arrow.arrowX = arrow.width - 10
                arrow.arrowY = arrow.height / 2 + 10
                arrow.open()
            }
        }
    }
//    Shape {
//        x: 20
//        y: 20
//        ShapePath {
//            fillColor: "red"
//            strokeColor: "transparent"
//            ArrowBoxPath {
//                id: arrow
//                width: 60
//                height: 150
//                arrowX: 0
//                arrowY: 0
//                arrowWidth: 50
//                arrowHeight: 30
//                roundedRadius: 8
//                spread: -5
//            }
//        }
//    }

//    Shape {
//        x: 20
//        y: 20
////        visible: false
//        ShapePath {
//            fillColor: "green"
//            strokeColor: "transparent"
//            ArrowBoxPath {
//                width: arrow.width
//                height: arrow.height
//                arrowDirection: arrow.arrowDirection
//                arrowX: arrow.arrowX
//                arrowY: arrow.arrowY
//                arrowWidth: arrow.arrowWidth
//                arrowHeight: arrow.arrowHeight
//                roundedRadius: arrow.roundedRadius
////                spread: 15
//            }
//        }
//    }
}
