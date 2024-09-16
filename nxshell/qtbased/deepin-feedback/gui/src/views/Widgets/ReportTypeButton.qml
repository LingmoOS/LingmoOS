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
    id:reportTypeButton
    width: 202
    height: 40
    state: actived ? "actived" : "normal"

    property bool actived: false
    property url iconPath: ""
    property alias textItem: text_item
    property alias text: text_item.text

    signal entered()
    signal exited()
    signal clicked()

    Rectangle {
        id: contentRec
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: 30
        radius: 3

        Image {
            id: typeIcon
            visible: reportTypeButton.actived
            source: iconPath
            width: 40
            height: 40
            anchors.bottom: parent.bottom
            anchors.right: text_item.left
        }

        Text {
            id:text_item
            anchors.centerIn: parent
            width: contentWidth
            height: parent.height
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 13
            clip: true
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onEntered: {
                reportTypeButton.entered()
            }

            onExited: {
                reportTypeButton.exited()
            }

            onClicked: {
                reportTypeButton.clicked()
            }
        }
    }

    states:[
        State {
            name: "normal"
            PropertyChanges {target: contentRec; color:bgNormalColor; border.color: buttonBorderColor}
            PropertyChanges {target: text_item; color: textNormalColor}
        },
        State {
            name: "actived"
            PropertyChanges {target: contentRec; color: bgActivedColor; border.color: bgActivedColor}
            PropertyChanges {target: text_item; color: textActivedColor}
        }
    ]
}
