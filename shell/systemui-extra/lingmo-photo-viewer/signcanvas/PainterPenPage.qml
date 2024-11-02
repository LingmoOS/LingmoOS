import QtQuick 2.12
import QtQuick.Window 2.12

Rectangle {
    id: root
    color: "transparent"
    objectName: "painterPenPage"
    property var imageTarget
    anchors.fill: imageTarget
    scale: imageTarget.scale
    rotation: imageTarget.rotation
    property int mouseqX: 0
    property int mouseqY: 0
    property var ctx
    property bool hasPaint: false
    property bool isMouseMoveEnable: false //是否允许鼠标移动绘制事件
    property bool hasReleased: true
    property var pointList: []
    property int changeNum: 0

    Canvas {
        id: canvas
        anchors.fill: parent

        property real lastX
        property real lastY

        onPaint: {
            //画曲线-自定义
            ctx = getContext('2d')
            if (!isMouseMoveEnable) {
                ctx.clearRect(0,0,width,height) //清空所画图形
                return
            }
            ctx.lineWidth = 1
            ctx.strokeStyle = variables.painterColor
            ctx.fillStyle = variables.painterColor
            ctx.beginPath()
            ctx.moveTo(lastX, lastY)
            ctx.arc(lastX,lastY,variables.painterThickness / 2,0,Math.PI* 2)
            ctx.fill();
            ctx.stroke()
            ctx.beginPath()
            ctx.moveTo(lastX, lastY)
            lastX = area.mouseX
            lastY = area.mouseY
            ctx.lineWidth = variables.painterThickness
            ctx.lineTo(lastX, lastY)
            ctx.stroke()

        }
        MouseArea {
            id: area
            anchors.fill: parent
            enabled: true
            onPressed: {
                if (variables.painterType != 4) {
                    return
                }
                if (!hasReleased) {
                    return
                }
                //画笔画线
                canvas.lastX = mouseX
                canvas.lastY = mouseY
                isMouseMoveEnable = true
            }
            onPositionChanged: {
                if (variables.painterType != 4) {
                    return
                }
                if (!hasReleased) {
                    return
                }
                pointList[changeNum++] = Qt.point(mouseX,mouseY)
                //画笔画线
                hasPaint = true
                canvas.requestPaint()
            }
            onReleased: {
                variables.painterPencilPaint(pointList)
                hasReleased = false
                area.enabled = false
                variables.painterRelease = !variables.painterRelease

                pointList = []
                changeNum = 0
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
