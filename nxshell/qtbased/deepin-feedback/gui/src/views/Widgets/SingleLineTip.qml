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

DWindow{
    id: rootwindow

    property int radius: 3
    property int borderWidth: 0
    property int arrowWidth: 8
    property int arrowHeight: 6
    property int arrowLeftMargin: 15
    property int destroyInterval: 200
    property int fontPixelSize: 12
    property color shadowColor
    property color borderColor: "#ced0d3"
    property color backgroundColor: Qt.rgba(0,0,0,0.9)
    property color textColor: "#ff8c03"
    property string toolTip
    property var showTip: showTipAtLeft

    width: 100
    height: 26
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.ToolTip | Qt.WindowStaysOnTopHint

    function showTipAtLeft(){
        arrow.y = arrowLeftMargin
        bg.width = width-arrow.width
        bg.height = height
        bg.x = arrow.width
        arrow.begin_p = Qt.point(arrow.width, 0)
        arrow.center_p = Qt.point(0, arrow.height/2)
        arrow.end_p = Qt.point(arrow.width, arrow.height)
        showTip = showTipAtLeft;
        visible = true
    }

    function showTipAtRight(){
        arrow.y = arrowLeftMargin
        bg.width = width-arrow.width
        arrow.x = bg.width
        bg.height = height
        arrow.begin_p = Qt.point(0, 0)
        arrow.center_p = Qt.point(arrow.width, arrow.height/2)
        arrow.end_p = Qt.point(0, arrow.height)
        showTip = showTipAtRight;
        visible = true
    }

    function showTipAtTop(){
        arrow.x = arrowLeftMargin
        bg.width = width
        bg.y = arrow.height
        bg.height = height - arrow.height
        arrow.begin_p = Qt.point(0, arrow.height)
        arrow.center_p = Qt.point(arrow.width/2, 0)
        arrow.end_p = Qt.point(arrow.width, arrow.height)
        showTip = showTipAtTop;
        visible = true
    }

    function showTipAtBottom(){
        arrow.x = arrowLeftMargin
        bg.width = width
        bg.height = height - arrow.height
        arrow.y = bg.height
        arrow.begin_p = Qt.point(0, 0)
        arrow.center_p = Qt.point(arrow.width/2, arrow.height)
        arrow.end_p = Qt.point(arrow.width, 0)
        showTip = showTipAtBottom;
        visible = true
    }

    function destroyTip(){
        timer.start()
    }

    Canvas{
        id: arrow

        property point begin_p
        property point center_p
        property point end_p

        width: arrowWidth
        height: arrowHeight

        onPaint: {
            var ctx = arrow.getContext('2d');
            ctx.strokeStyle = backgroundColor
            ctx.fillStyle = backgroundColor
            ctx.beginPath();
            ctx.moveTo(begin_p.x, begin_p.y);
            ctx.lineTo(center_p.x, center_p.y)
            ctx.lineTo(end_p.x, end_p.y);
            ctx.stroke();
            ctx.fill();
        }
    }

    Rectangle{
        id: bg

        color: backgroundColor
        border{
            width: borderWidth
            color: borderColor
        }
        radius: rootwindow.radius

        Text{
            id: text

            text: toolTip
            color: textColor
            anchors.centerIn: parent
            wrapMode: Text.WrapAnywhere
            width: parent.width-10
            elide: Text.ElideRight
            lineHeightMode: Text.FixedHeight
            lineHeight: fontPixelSize*1.2
            font.pixelSize: fontPixelSize
            height: Math.min(lineHeight*3, parent.height-10)
        }
    }

    Timer{
        id: timer

        interval: destroyInterval>0?destroyInterval:0
        repeat: false
        onTriggered: {
            rootwindow.close()
        }
    }
}

