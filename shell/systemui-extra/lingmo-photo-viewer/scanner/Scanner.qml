import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.12

Rectangle {
    objectName: "scannerRect"
    id: scannerWindow
    visible: true
    color: "transparent"

    ScannerVariables {id: scannerVariables}
    ScannerBaseFunction {id: scannerBasefunction}

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton|Qt.RightButton|Qt.MiddleButton
        hoverEnabled: true
        cursorShape:  Qt.ArrowCursor

        property int angleDeltaX: 0
        property int angleDeltaY: 0

        onWheel: {
            if (wheel.angleDelta.y % 120 === 0) {
                scannerVariables.imageZoomSpeed = 15 //缩放速度
            } else {
                scannerVariables.imageZoomSpeed = 1
            }

            angleDeltaX += wheel.angleDelta.x
            angleDeltaY += wheel.angleDelta.y  //累加垂直增量
            var wheelDelta = Qt.point(angleDeltaX, angleDeltaY)

            angleDeltaX = 0 //记录滚轮增量后置零
            angleDeltaY = 0

            if (scannerVariables.operateImageWay != 10) { //裁剪
               scannerVariables.wheelChange = !scannerVariables.wheelChange
            }

            if (wheelDelta.y > 0) {
                //放大图片
                scannerArea.zoomIn(wheelDelta)
            } else {
                //缩小图片
                scannerArea.zoomOut(wheelDelta)
            }
        }
    }

    Image {
        id: scannerBgImage
        anchors.fill: parent

        MouseArea {
            id: mousemouse
            anchors.fill: parent
            hoverEnabled: true

            onPositionChanged: {
                scannerVariables.mousePos = Qt.point(mouseX, mouseY)
            }
        }
    }

    ScannerImageArea {
        objectName: "scannerImageItem"
        id: scannerArea
        x: 0
        y: 0
        width: scannerWindow.width
        height: scannerWindow.height
    }

    onWidthChanged: {
        scannerVariables.curWidW = width
    }
    onHeightChanged: {
        scannerVariables.curWidH = height
    }

    function setOperateWay(way) {
        scannerVariables.operateImageWay = way
    }
}
