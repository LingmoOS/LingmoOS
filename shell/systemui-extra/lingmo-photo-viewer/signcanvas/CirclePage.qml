import QtQuick 2.12
import QtQuick.Window 2.12

Rectangle {
    id: root
    color: "transparent"
    objectName: "circlePage"
    property var imageTarget
    anchors.fill: imageTarget
    scale: imageTarget.scale
    rotation: imageTarget.rotation

    //鼠标点击坐标位置---画矩形
    property real startX :0    //储存鼠标开始时的坐标
    property real startY :0
    property real stopX  :0   //储存鼠标结束时的坐标
    property real stopY :0
    property bool isMouseMoveEnable: false //是否允许鼠标移动绘制事件
    property var ctx
    property bool hasReleased: true
    property bool hasPaint: false

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            //画圆
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

            //画圆/椭圆
            ctx.ellipse(startX,startY,(stopX-startX) ,(stopY-startY),false)
            ctx.stroke()

        }
        MouseArea {
            id: area
            anchors.fill: parent
            enabled: true
            onPressed: {
                if (variables.painterType != 1) {
                    return
                }
                if (!hasReleased) {
                    return
                }
                //画圆
                startX = mouse.x
                startY = mouse.y
                isMouseMoveEnable = true

            }
            onPositionChanged: {
                if (variables.painterType != 1) {
                    return
                }
                if (!hasReleased) {
                    return
                }
                //画圆
                if (isMouseMoveEnable){
                    hasPaint = true
                    canvas.requestPaint()   //绘制函数
                }
            }
            onReleased: {
                if (stopX != 0 || stopY != 0) {
                    variables.painterCircleSignal(startX,startY,(stopX-startX) ,(stopY-startY))
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
