/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
import QtQuick 2.1
import QtGraphicalEffects 1.0

Rectangle {
    id: rect
    color: "transparent"

    width: 22
    height: 23

    property int topLeftRadius: 3
    property int topRightRadius: 3
    property int bottomLeftRadius: 3
    property int bottomRightRadius: 3

    property int outsideBorderWidth: 1
    property color outsideBorderColor: "#26000000"

    property int insideBorderTopWidth: 1
    property int insideBorderBottomWidth: 1
    property color insideBorderTopColor: "#33ffffff"
    property color insideBorderBottomColor: "#19ffffff"

    property bool isPress: false
    property bool isHover: false

    signal clicked
    signal entered
    signal exited
    signal pressed
    signal released

    QtObject {
        id: outsideBorder
        property point topLeftPoint: Qt.point(0,0)
        property point topRightPoint: Qt.point(rect.width,0)
        property point bottomLeftPoint: Qt.point(0,rect.height - 1)
        property point bottomRightPoint: Qt.point(rect.width,rect.height - 1)
    }

    QtObject {
        id: insideBorder
        property point topLeftPoint: Qt.point(outsideBorderWidth,outsideBorderWidth)
        property point topRightPoint: Qt.point(rect.width - outsideBorderWidth,outsideBorderWidth)
        property point bottomLeftPoint: Qt.point(outsideBorderWidth,rect.height - outsideBorderWidth - 1)
        property point bottomRightPoint: Qt.point(rect.width - outsideBorderWidth,rect.height - outsideBorderWidth - 1)
    }


    Canvas {
        id: canvas
        width: parent.width
        height: parent.height - 1
        anchors.centerIn: parent

        contextType: "2d"

        Path {
            id: outsidePath
            startX: outsideBorder.topLeftPoint.x
            startY: outsideBorder.topLeftPoint.y + topLeftRadius

            PathArc {
                x: outsideBorder.topLeftPoint.x + topLeftRadius
                y: outsideBorder.topLeftPoint.y
                radiusX: topLeftRadius
                radiusY: topLeftRadius
                useLargeArc: false
            }
            PathLine { x: outsideBorder.topRightPoint.x - topRightRadius; y: outsideBorder.topRightPoint.y }
            PathArc {
                x: outsideBorder.topRightPoint.x
                y: outsideBorder.topRightPoint.y + topRightRadius
                radiusX: topRightRadius
                radiusY: topRightRadius
                useLargeArc: false
            }
            PathLine { x: outsideBorder.bottomRightPoint.x; y: outsideBorder.bottomRightPoint.y - bottomRightRadius }
            PathArc {
                x: outsideBorder.bottomRightPoint.x - bottomRightRadius
                y: outsideBorder.bottomRightPoint.y
                radiusX: bottomRightRadius
                radiusY: bottomRightRadius
                useLargeArc: false
            }
            PathLine { x: outsideBorder.bottomLeftPoint.x + bottomLeftRadius; y: outsideBorder.bottomLeftPoint.y}
            PathArc {
                x: outsideBorder.bottomLeftPoint.x
                y: outsideBorder.bottomLeftPoint.y - bottomLeftRadius
                radiusX: bottomLeftRadius
                radiusY: bottomLeftRadius
                useLargeArc: false
            }
            PathLine { x: outsideBorder.topLeftPoint.x; y: outsideBorder.topLeftPoint.y + topLeftRadius}
        }


        Path {
            id: insideTopPath
            startX: insideBorder.topLeftPoint.x
            startY: insideBorder.topLeftPoint.y + topLeftRadius - outsideBorderWidth

            PathArc {
                x: insideBorder.topLeftPoint.x + topLeftRadius - outsideBorderWidth
                y: insideBorder.topLeftPoint.y
                radiusX: topLeftRadius - outsideBorderWidth
                radiusY: topLeftRadius - outsideBorderWidth
                useLargeArc: false
            }
            PathLine { x: insideBorder.topRightPoint.x - topRightRadius + outsideBorderWidth; y: insideBorder.topRightPoint.y }
            PathArc {
                x: insideBorder.topRightPoint.x
                y: insideBorder.topRightPoint.y + topRightRadius - outsideBorderWidth
                radiusX: topRightRadius - outsideBorderWidth
                radiusY: topRightRadius - outsideBorderWidth
                useLargeArc: false
            }
        }

        Path {
            id: insideBottomPath
            startX: insideBorder.topRightPoint.x
            startY: insideBorder.topRightPoint.y + topRightRadius - outsideBorderWidth

            PathLine { x: insideBorder.bottomRightPoint.x; y: insideBorder.bottomRightPoint.y - bottomRightRadius  + outsideBorderWidth}
            PathArc {
                x: insideBorder.bottomRightPoint.x - bottomRightRadius + outsideBorderWidth
                y: insideBorder.bottomRightPoint.y
                radiusX: bottomRightRadius - outsideBorderWidth
                radiusY: bottomRightRadius - outsideBorderWidth
                useLargeArc: false
            }
            PathLine { x: insideBorder.bottomLeftPoint.x + bottomLeftRadius - outsideBorderWidth; y: insideBorder.bottomLeftPoint.y}
            PathArc {
                x: insideBorder.bottomLeftPoint.x
                y: insideBorder.bottomLeftPoint.y - bottomLeftRadius + outsideBorderWidth
                radiusX: bottomLeftRadius - outsideBorderWidth
                radiusY: bottomLeftRadius - outsideBorderWidth
                useLargeArc: false
            }
            PathLine { x: insideBorder.topLeftPoint.x; y: insideBorder.topLeftPoint.y + topLeftRadius - outsideBorderWidth}
        }

        onPaint: {
            if(!context){
                return
            }
            context.clearRect(0,0,canvas.width,canvas.height)
            //fill frame
            var gradient = context.createLinearGradient(outsideBorder.topLeftPoint.x, outsideBorder.topLeftPoint.y
                                                        ,outsideBorder.bottomLeftPoint.x,outsideBorder.bottomLeftPoint.y);
            gradient.addColorStop(0,isPress ? "#68c3ff": isHover ? "#D4EFFF" : "#ffffff");
            gradient.addColorStop(1,isPress ? "#68c3ff" : isHover ? "#9AD7FF" : "#fafafa");

            context.fillStyle = gradient;
            context.path = outsidePath;
            context.lineWidth = outsideBorderWidth;
            context.fill();

            //draw the inside top border
            context.strokeStyle = insideBorderTopColor;
            context.path = insideTopPath;
            context.lineWidth = insideBorderTopWidth;
            context.stroke();

            //draw the inside bottom border
            context.strokeStyle = insideBorderBottomColor;
            context.path = insideBottomPath;
            context.lineWidth = insideBorderBottomWidth;
            context.stroke();

            //draw the outside border
            context.strokeStyle = outsideBorderColor;
            context.path = outsidePath;
            context.lineWidth = outsideBorderWidth;
            context.stroke();
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            rect.clicked()
        }
        onPressed: {
            rect.isPress = true
            canvas.requestPaint()
            rect.pressed()
        }
        onReleased: {
            rect.isPress = false
            canvas.requestPaint()
            rect.released()
        }
        onEntered: {
            rect.isHover = true
            canvas.requestPaint()
            rect.entered()
        }
        onExited: {
            rect.isHover = false
            canvas.requestPaint()
            rect.exited()
        }
    }

    DropShadow {
        anchors.fill: canvas
        // fast: false
        horizontalOffset: 0
        verticalOffset: 1
        radius: 0
        samples: 16
        color: "#02000000"
        source: canvas
    }
}
