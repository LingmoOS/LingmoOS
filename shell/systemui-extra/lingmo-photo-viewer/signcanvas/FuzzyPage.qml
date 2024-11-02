import QtQuick 2.0
import QtGraphicalEffects 1.0
Rectangle{
    id:root1
    clip: true
    objectName: "fuzzyPage"
    color: "transparent"
    anchors.fill: imageTarget
    scale: imageTarget.scale
    // 属性
    property Item imageTarget  // 模糊源
    property bool hasPaint: false
    property rect blurRect

    property real startX: 0     //储存鼠标开始时的坐标
    property real startY: 0
    property real stopX: 0      //储存鼠标结束时的坐标
    property real stopY: 0

    // 毛玻璃效果
    Rectangle {
        id: root
        objectName: "fuzzyPage1"
        x:startX
        y:startY
        width: stopX - startX
        height: stopY - startY
        color: "transparent"
        clip:true

        FastBlur {
            id: blur
            source: parent.parent.imageTarget
            x: -startX
            y: -startY
            width: source.width
            height: source.height
            radius: 30
        }
    }

    function acturalType() {
        return root1.objectName
    }
    function clearMask (){
        root1.destroy()
    }

    function getRect() {
        blurRect.x = startX
        blurRect.y = startY
        blurRect.width = root.width
        blurRect.height = root.height
        return blurRect
    }

    MouseArea{
        id: area
        anchors.fill: parent
        onPressed: {
            if (variables.painterType != 7) {
                return
            }
            startX = mouseX
            startY = mouseY
            stopX = mouseX
            stopY = mouseY
        }
        onPositionChanged: {
            if (variables.painterType != 7) {
                return
            }
            stopX = mouseX
            stopY = mouseY
            hasPaint = true
        }
        onReleased: {
            variables.painterBlurPaint(getRect())
            imageTarget.paintCanvas()
        }
    }
}
