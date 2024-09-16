/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.1

Item {
    width: parent.width
    height: 16

    Image {
        id: appIcon
        width: 16
        height: 16
        anchors.left: parent.left
        anchors.top: parent.top
        source: "qrc:///views/Widgets/images/deepin-feedback.png"
    }

    Text {
        id: appTitleText
        color: "#999999"
        font.pixelSize: 12
        text: dsTr("Deepin User Feedback")
        verticalAlignment: Text.AlignVCenter
        anchors {left: appIcon.right; leftMargin: 4; verticalCenter: appIcon.verticalCenter}
    }

}
