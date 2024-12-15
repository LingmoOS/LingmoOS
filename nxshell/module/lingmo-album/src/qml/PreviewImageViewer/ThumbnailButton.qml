// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4

ToolButton {



    property color clr_enter: "#dcdcdc"
    property color clr_exit: "#ffffff"
    property color clr_click: "#aba9b2"
    property color clr_release: "#ffffff"

    //自定义点击信号
    signal clickedLeft()
    signal clickedRight()
    signal release()

    height: 50
    width: 50
    icon.width: 40; icon.height: 40
    background: Rectangle{
        id :buttonrect
        radius: 8
//        color: tool.enabled ? "#ffffff" : "#ffffff"

    }
//        Image {
//            id: icon
//            width: parent.width
//            height: parent.height
//            source: ""
//            fillMode: Image.PreserveAspectFit
//            clip: true
//            anchors.top: parent.top
//            anchors.right: parent.right
//            anchors.left: parent.left
//            anchors.margins: 0
//            smooth: true
//        }
//        Text {
//            id: button
//    //        text: qsTr("button")

//            anchors.top: icon.bottom
//            anchors.topMargin: 5
//            anchors.horizontalCenter: icon.horizontalCenter
//            anchors.bottom: icon.bottom
//            anchors.bottomMargin: 5

//            font.bold: true
//            font.pointSize: 14
//        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true

            //接受左键和右键输入
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: {
                //左键点击
                if (mouse.button === Qt.LeftButton) {
                    parent.clickedLeft()
                } else if(mouse.button === Qt.RightButton) {
                    parent.clickedRight()
                }
            }

            //按下
            onPressed: {
                buttonrect.color = clr_click
            }

            //释放
            onReleased: {
                buttonrect.color = clr_enter
                parent.release()
            }

            //指针进入
            onEntered: {
                buttonrect.color = clr_enter
            }

            //指针退出
            onExited: {
                buttonrect.color = clr_exit
            }
        }
    }


