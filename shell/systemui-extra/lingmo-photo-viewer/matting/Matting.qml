import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.12

Rectangle {
    objectName: "mattingRect"
    id: mattingWindow
    visible: true
    signal closeChildWindow()
    MattingVariables {id: mattingvariables}
    MattingBaseFunction {id: mattingbasefunction}

    onWidthChanged: {
        mattingvariables.curWidW = width
    }
    onHeightChanged: {
        mattingvariables.curWidH = height
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton|Qt.RightButton|Qt.MiddleButton
        hoverEnabled: true
        cursorShape:  Qt.ArrowCursor

        property int angleDeltaX: 0
        property int angleDeltaY: 0

        onClicked: {
            closeChildWindow()
        }

        onWheel: {
            if (wheel.angleDelta.y % 120 === 0) {
                mattingvariables.imageZoomSpeed = 15
            } else {
                mattingvariables.imageZoomSpeed = 1
            }

            angleDeltaX += wheel.angleDelta.x
            angleDeltaY += wheel.angleDelta.y
            var wheelDelta = Qt.point(angleDeltaX, angleDeltaY)

            angleDeltaX = 0
            angleDeltaY = 0

            if (wheelDelta.y > 0) {
                //放大图片
                mattingArea.zoomIn(wheelDelta)
            } else {
                //缩小图片
                mattingArea.zoomOut(wheelDelta)
            }
        }
    }

    Image {
        id: mattingBgImage
        anchors.fill: parent
        MouseArea {
            id: mousemouse
            anchors.fill: parent
            hoverEnabled: true
            onReleased: {

            }

            onPressed: {
                startPressPoint = Qt.point(mouseX, mouseY)
            }

            onPositionChanged: {
                mattingvariables.mousePos = Qt.point(mouseX, mouseY)
            }
            onMouseXChanged: {
                mattingvariables.mouseCurPos = Qt.point(mouseX, mouseY)
            }
            onMouseYChanged: {
                mattingvariables.mouseCurPos = Qt.point(mouseX, mouseY)
            }
        }
    }

    MattingImageArea {
        objectName: "mattingImageItem"
        id: mattingArea
        x: 0
        y: 0
        width: mattingWindow.width
        height: mattingWindow.height
    }

    function setOperateWay(way) {
        mattingvariables.operateImageWay = way
    }
}




