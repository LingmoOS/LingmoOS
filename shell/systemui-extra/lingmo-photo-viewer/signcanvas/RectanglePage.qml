import QtQuick 2.12
import QtQuick.Window 2.12

Rectangle {
    id: root
    color: "transparent"
    objectName: "rectanglePage"

    property var imageTarget
    anchors.fill: imageTarget
    scale: imageTarget.scale
    rotation: imageTarget.rotation

    //鼠标点击坐标位置---画矩形
    property real startX: 0     //储存鼠标开始时的坐标
    property real startY: 0
    property real stopX: 0      //储存鼠标结束时的坐标
    property real stopY: 0
    property bool isMouseMoveEnable: false //是否允许鼠标移动绘制事件
    property var ctx
    property bool hasReleased: true
    property bool hasPaint: false

    property double rectWidth: 0
    property double rectHeight: 0


    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            //画矩形方框
            ctx = getContext("2d")
            ctx.lineWidth = variables.painterThickness
            ctx.strokeStyle = variables.painterColor

            if (!isMouseMoveEnable) {
                ctx.clearRect(0,0,width,height) //清空所画图形
                return
            }
            if (isMouseMoveEnable){
                ctx.clearRect(0,0,width,height)
            }
            //开始绘制
            ctx.beginPath()
            ctx.moveTo(startX,startY)

            //记录移动终点
            stopX = area.mouseX
            stopY = area.mouseY

            //绘制长方形
            ctx.strokeRect(startX,startY,stopX-startX,stopY-startY)
            ctx.stroke()

        }
        MouseArea {
            id: area
            anchors.fill: parent
            enabled: true

            onPressed: {
                root.forceActiveFocus()
                if (variables.painterType != 0 ) {
                    return
                }
                if (!hasReleased) {
                    return
                }

                //画矩形
                startX = mouse.x
                startY = mouse.y
                isMouseMoveEnable = true

            }
            onPositionChanged: {
                if (variables.painterType != 0) {
                    return
                }
                if (!hasReleased) {
                    return
                }
                //画矩形
                if (isMouseMoveEnable){
                    hasPaint = true
                    canvas.requestPaint()   //绘制函数
                }

            }
            onReleased: {
                if (stopX != 0 || stopY != 0) {
                    variables.painterRectSignal(Qt.rect(startX,startY,stopX-startX,stopY-startY))
                }
                hasReleased = false
                area.enabled = false
                variables.painterRelease = !variables.painterRelease

            }
        }

    }
    function clearMask (){
        root.destroy()
    }
    function acturalType() {
        return root.objectName
    }
}
