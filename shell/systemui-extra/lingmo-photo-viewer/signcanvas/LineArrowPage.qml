import QtQuick 2.12
import QtQuick.Window 2.12

Rectangle {
    id: root
    color: "transparent"
    objectName: "LineArrowPage"
    property var imageTarget
    anchors.fill: imageTarget
    scale: imageTarget.scale
    rotation: imageTarget.rotation
    property bool hasPaint: false


    //鼠标点击坐标位置---画矩形
    property real startX:0     //储存鼠标开始时的坐标
    property real startY:0
    property real stopX :0     //储存鼠标结束时的坐标
    property real stopY:0
    property bool isMouseMoveEnable: false //是否允许鼠标移动绘制事件
    property bool hasReleased: true

    property var ctx
    property point arrowStart
    property point arrowEnd
    property point lineStop

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            //画矩形方框
            ctx = getContext("2d")
            ctx.lineWidth = variables.painterThickness
            //设置边框色
            ctx.strokeStyle = variables.painterColor;
            // 设置填充色
            ctx.fillStyle = variables.painterColor

            if (!isMouseMoveEnable) {
                ctx.clearRect(0,0,width,height) //清空所画图形
                return
            }
            if (isMouseMoveEnable){
                ctx.clearRect(0,0,width,height)
            }
            //记录移动终点
            stopX = area.mouseX
            stopY = area.mouseY
            const dx = stopX - startX
            const dy = stopY - startY
//            const headlen = Math.sqrt(dx * dx + dy * dy) * 0.1
            const headlen = variables.painterThickness * 6
            const angle = Math.atan2(dy, dx);

            ctx.lineWidth = 1
            //三角箭头
            ctx.beginPath()
            ctx.moveTo(stopX - headlen * Math.cos(angle - Math.PI / 6), stopY - headlen * Math.sin(angle - Math.PI / 6))
            ctx.lineTo(stopX, stopY )
            ctx.lineTo(stopX - headlen * Math.cos(angle + Math.PI / 6), stopY - headlen * Math.sin(angle + Math.PI / 6))
            ctx.lineTo(stopX - headlen * Math.cos(angle - Math.PI / 6), stopY - headlen * Math.sin(angle - Math.PI / 6))
            ctx.closePath()
            //赋值
            arrowStart = Qt.point(stopX - headlen * Math.cos(angle - Math.PI / 6), stopY - headlen * Math.sin(angle - Math.PI / 6))
            arrowEnd = Qt.point(stopX - headlen * Math.cos(angle + Math.PI / 6), stopY - headlen * Math.sin(angle + Math.PI / 6))
            //使用之前设置好的填充色进行填充
            ctx.fill()
            ctx.stroke()
            //绘制线段
            var a1 = (arrowStart.y - arrowEnd.y) / (arrowStart.x - arrowEnd.x)
            var b1 = arrowStart.y - a1 * arrowStart.x
            var a2 =(startY - stopY) /(startX - stopX)
            var b2 = startY - a2 * startX
            lineStop = Qt.point((b2-b1) / (a1-a2),(b2-b1) / (a1-a2) * a1 + b1)
            ctx.lineWidth = variables.painterThickness
            ctx.beginPath()
            ctx.moveTo(startX,startY)
            ctx.lineTo(lineStop.x,lineStop.y)
            ctx.stroke()
        }
        MouseArea {
            id: area
            anchors.fill: parent
            enabled: true
            onPressed: {
                if (variables.painterType != 3) {
                    return
                }
                if (!hasReleased) {
                    return
                }
                //起始点
                startX = mouse.x
                startY = mouse.y
                isMouseMoveEnable = true

            }
            onPositionChanged: {
                if (variables.painterType != 3) {
                    return
                }
                if (!hasReleased) {
                    return
                }
                //画图
                if (isMouseMoveEnable){
                    hasPaint = true
                    canvas.requestPaint()   //绘制函数
                }

            }
            onReleased: {
                if (stopX != 0 || stopY != 0) {
                    variables.painterArrowPaint(Qt.point(startX,startY),Qt.point(mouseX,mouseY),arrowStart,arrowEnd)
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


