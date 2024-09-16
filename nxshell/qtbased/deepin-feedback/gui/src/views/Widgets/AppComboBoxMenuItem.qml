/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.1
import Deepin.Widgets 1.0

Item {
    id: wrapper
    width: wrapper.ListView.view.width; height: 26

    signal selectAction(int index)
    signal enter(int index)

    property alias text: label.text

    Rectangle {
        color: wrapper.ListView.view.selectIndex == index ? "#5498ec" : bgNormalColor
        anchors.fill: parent
    }

    Image {
        id:draftImg
        visible: mainObject.draftNotEmpty(getProjectIDByName(label.text))
        width: 12
        height: 12
        source: "images/draft.png"
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.verticalCenter: parent.verticalCenter
    }

    DLabel {
        id: label
        anchors.left: draftImg.right
        anchors.leftMargin: 3
        anchors.verticalCenter: parent.verticalCenter
        text: "text " + index
        font.pixelSize: 12
        font.family: label.text
        color: wrapper.ListView.view.selectIndex == index ? "#ffffff" : textNormalColor
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered:{
            wrapper.ListView.view.selectIndex = index
            enter(index)
        }
        onExited: {
            wrapper.ListView.view.selectIndex = -1
        }
        onClicked: selectAction(index)
    }
}
