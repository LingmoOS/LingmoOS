/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.1

ButtonFrame {
    width: 60
    height: 30

    property alias textItem: text_item
    property alias text: text_item.text

    onEntered: {
        text_item.color = "#000000"
    }
    onExited: {
        text_item.color = textNormalColor
    }

    Text {
        id:text_item
        anchors.centerIn: parent
        width: contentWidth + 40
        height: contentHeight
        wrapMode: Text.Wrap
        color: textNormalColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 13
        clip: true
    }
}

