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
    id:buttonToolTip
    width: 0
    clip: true

    property int animationDuration: 300
    property int maxWidth: 214
    property int autoHideInterval: -1
    property int delayShowInterval: -1
    property alias textItem: tipText

    function showTip(message){
        delayHideTimer.stop()
        tipText.color = "#ff8d24"
        tipText.text = message
        buttonToolTip.width = tipText.contentWidth > maxWidth ? maxWidth : tipText.contentWidth
        tipText.wrapMode = tipText.contentWidth > maxWidth ? Text.WordWrap : Text.NoWrap
        delayHideTimer.start()
    }

    function showTipWithColor(message, color){
        tipText.color = color
        delayHideTimer.stop()
        tipText.text = message
        buttonToolTip.width = tipText.contentWidth > maxWidth ? maxWidth : tipText.contentWidth
        tipText.wrapMode = tipText.contentWidth > maxWidth ? Text.WordWrap : Text.NoWrap
        delayHideTimer.start()
    }

    function hideTip(){
        buttonToolTip.width = 0
    }

    Timer {
        id: delayHideTimer
        interval: autoHideInterval
        onTriggered: hideTip()
    }

    Behavior on width {
        PropertyAnimation {duration: animationDuration;easing.type : Easing.OutQuart}
    }

    Text {
        id:tipText
        color: "#ff8d24"
        font.pixelSize: 12
        clip: true
        width: parent.width
        height: parent.height
        horizontalAlignment: Text.AlignLeft
    }
}
