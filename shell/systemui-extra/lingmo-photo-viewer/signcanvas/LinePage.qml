import QtQuick 2.12
import QtQuick.Window 2.12
//mark条
Rectangle {
    id: root
    color: "transparent"
    objectName: "linePage"
    property var imageTarget
    anchors.fill: imageTarget
    scale: imageTarget.scale
    rotation: imageTarget.rotation
    property bool hasPaint: false


    //鼠标点击坐标位置---画矩形
    property real startX: 0    //储存鼠标开始时的坐标
    property real startY: 0
    property real stopX: 0    //储存鼠标结束时的坐标
    property real stopY: 0
    property bool isMouseMoveEnable: false //是否允许鼠标移动绘制事件
    property var ctx
    property bool hasReleased: true
    property double globalAlpha: 0.3

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            //画矩形方框
            ctx = getContext("2d")
            ctx.lineWidth = variables.painterThickness
            ctx.strokeStyle = variables.painterColor
            //设置透明度
            ctx.globalAlpha = globalAlpha

            if (!isMouseMoveEnable) {
                ctx.clearRect(0,0,width,height) //清空所画图形
                return;
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

            //绘制直线
            ctx.lineTo(stopX,stopY)
            ctx.stroke()

        }
        MouseArea {
            id: area
            anchors.fill: parent
            onPressed: {

                if (variables.painterType != 5) {
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
                if (variables.painterType != 5) {
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
                    variables.painterMarkPaint(Qt.point(startX,startY),Qt.point(mouseX,mouseY),globalAlpha)
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
