/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.1
import QtQuick.Window 2.1
import Deepin.Widgets 1.0

DPopupWindow {
    id: menuPopupWindow
    property int frameEdge: menuFrame.shadowRadius + menuFrame.frameRadius
    property int minWidth: 30
    property real posX: 0
    property real posY: 0

    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.X11BypassWindowManagerHint

    x: posX - 28
    y: posY - 12

    width: minWidth + 24
    height: completeViewBox.height + 32

    property int maxHeight: -1

    property alias currentIndex: completeView.selectIndex
    property var labels
    visible: false

    signal menuSelect(int index)
    signal menuEnter(int index)

    function menuUp(){
        if (completeView.selectIndex > 0)
            completeView.selectIndex --
    }

    function menuDown(){
        if (completeView.selectIndex < labels.length - 1)
            completeView.selectIndex ++
    }

    function selectMenu(){
        if(completeView.selectIndex>=0&&completeView.selectIndex<=completeView.count){
            menuPopupWindow.visible = false
            menuPopupWindow.menuSelect(completeView.selectIndex)
        }
    }

    DWindowFrame {
        id: menuFrame
        anchors.fill: parent
        frame.color: bgNormalColor
        frame.border.color: "#26000000"
        frameRadius: 3
        layer.enabled: true

        Item {
            id: completeViewBox
            anchors.centerIn: parent
            width: parent.width - 6
            height: completeView.height

            ListView {
                id: completeView
                width: parent.width
                height: maxHeight != -1 ? Math.min(childrenHeight, maxHeight) : childrenHeight
                maximumFlickVelocity: 1000
                model: labels

                property int selectIndex: 0
                property int childrenHeight: childrenRect.height

                delegate: AppComboBoxMenuItem {
                    text: labels[index]
                    onSelectAction:{
                        menuPopupWindow.visible = false
                        menuPopupWindow.menuSelect(index)
                    }
                    onEnter: menuPopupWindow.menuEnter(index)
                }
                clip: true
            }

            DScrollBar {
                flickable: completeView
            }
        }

    }

}
