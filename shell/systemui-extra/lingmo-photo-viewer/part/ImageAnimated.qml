/**************************************************************************
 **                                                                      **
 ** Copyright (C) 2011-2021 Lukas Spies                                  **
 ** Contact: http://photoqt.org                                          **
 **                                                                      **
 ** This file is part of PhotoQt.                                        **
 **                                                                      **
 ** PhotoQt is free software: you can redistribute it and/or modify      **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation, either version 2 of the License, or    **
 ** (at your option) any later version.                                  **
 **                                                                      **
 ** PhotoQt is distributed in the hope that it will be useful,           **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with PhotoQt. If not, see <http://www.gnu.org/licenses/>.      **
 **                                                                      **
 **************************************************************************/

import QtQuick 2.9

Item {
    objectName: "movieItemLower"
    id: imageNormal
    x: parent.width/2 - width/2
    y: parent.height/2 - height/2
    width: container.width
    height: container.height
    property real defaultScale: 1.0
    signal sendScale(var scale)
    property bool needWheelReact: true //是否需要滑轮响应
    property bool newImageLoaded: false //新图片加载
    property string imagePath: ""
    signal sendPos(var xPos,var yPos)
    property double imageVisibleLeftUpX: 0 //可视区域左上角的点在原图上的位置
    property double imageVisibleLeftUpY: 0
    property double imageLeftUpX: 0  //缩放后的图片的左上角相对于窗口的位置
    property double imageLeftUpY: 0
    signal needShowNavigator(var scale, var leftUpImageX, var leftUpImageY, var visibleWidth, var visibleHeight)
    signal closeNavigator()
    property bool imageNeedCenterZoom: true
    property bool mouseareaEnabeled: false
    property int mouseDragType: -1
    property bool startAni: true
    signal buriedPoint()

    AnimatedImage {
        id: theImage
        objectName: "image"
        property real curX: imageNormal.width/2 - width/2
        property real curY: imageNormal.height/2 - height/2
        property real realCurX
        property real realCurY
        property int index: parent.parent.parent.itemIndex
        x: curX
        y: curY
        focus: true
        //原始图像不做处理
        fillMode: Image.Pad
        clip: true
        cache: false
        asynchronous: true
        source: parent.parent.parent.imageUrl
        mirror: false
        //图像在缩放或变换时是否平滑过滤。平滑过滤可以提供更好的视觉质量
        smooth: (width * scale > variables.curWidW || height > variables.curWidH)
        //图像在缩放或变换时是否使用mipmap过滤。与平滑相比，缩小比例时，Mipmap过滤提供了更好的视觉质量，但它可能会以性能为代价（无论是在初始化图像时还是在渲染期间）。
        mipmap: (scale < defaultScale || (scale < 0.8 && defaultScale < 0.8)) && (width > variables.curWidW || height > variables.curWidH)
        rotation: 0
        property real rotateTo: 0
        onRotateToChanged: {
            if (isMoiveRunning()) {
                return
            }
            rotation = rotateTo
        }
        onRotationChanged: {
            if (isMoiveRunning()) {
                return
            }
            if(!rotani.running) {
                rotateTo = rotation
            }
        }

        property real curScale: 1
        scale: curScale

        Behavior on x {
            PropertyAnimation {
                id: xani
                duration: variables.startAni ? variables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }

        Behavior on y {
            PropertyAnimation {
                id: yani
                duration: variables.startAni ? variables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }
        Behavior on rotation {
            PropertyAnimation {
                id: rotani
                duration: variables.startAni ? variables.imageAniDuration : 0
                onRunningChanged: {
                    if (!rotani.running) {
                        imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
                    }
                }
                easing.type: Easing.OutCubic
            }
        }
        Behavior on scale {
            PropertyAnimation {
                id: scaleani
                duration: variables.startAni ? variables.imageAniDuration : 0
                onRunningChanged: {
                    if (!scaleani.running) {
                        imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
                        imageBase.makeSureNavigatorNeedShow(theImage)
                    }
                }
                easing.type: Easing.OutCubic
            }
        }
        onXChanged: {
            if (isMoiveRunning()) {
                return
            }
            setDrogLimit()
        }
        onYChanged: {
            if (isMoiveRunning()) {
                return
            }
            setDrogLimit()
        }

        function sendScale() {
            return theImage.curScale
        }

        function changeScale(scaleValue) {
            if (isMoiveRunning()) {
                return
            }
            imageBase.setImageScaleWithImageUpdate(theImage,scaleValue)
        }

        function sendScaleFirstTime() {
            return imageBase.getImageScale(theImage)
        }

        //重设位置
        function imagePostionReset() {
            if (isMoiveRunning()) {
                return
            }
            imageBase.resetImagePostion(theImage)
        }

        onStatusChanged: {
            if (isMoiveRunning()) {
                return
            }
            if (theImage.status == Image.Ready) {
//                theImage.source = imageNormal.imagePath
                if (newImageLoaded) {
                    theImage.imagePostionReset()
                }
            }
        }
        onSourceChanged: {
            if (isMoiveRunning()) {
                return
            }
            variables.startAni = false
            theImage.imagePostionReset()
            variables.startAni = true
        }

        function setSource(imagePath) {
            if (isMoiveRunning()) {
                return
            }
            imageNormal.imagePath = imagePath
            theImage.source = imageNormal.imagePath
        }
        function changeImage(imageUrl,imageWidth,imageHeight) {
            if (isMoiveRunning()) {
                return
            }

            if (variables.operateImageWay === 12) {
                theImage.source = ""
                variables.startAni = false
                theImage.x = variables.curWidW / 2 - imageWidth / 2
                theImage.y = variables.curWidH / 2 - imageHeight / 2
                imageNormal.imagePath = imageUrl
                theImage.source = imageNormal.imagePath
                variables.startAni = true
            }
            if (variables.operateImageWay === 17) {
                theImage.source = ""
                variables.startAni = false
                theImage.x = variables.curWidW / 2 - imageWidth / 2 + imageWidth + 10
                theImage.y = variables.curWidH / 2 - imageHeight / 2
                imageNormal.imagePath = imageUrl
                theImage.source = imageNormal.imagePath
                variables.startAni = true
                theImage.x = variables.curWidW / 2 - imageWidth / 2
            }
            if (variables.operateImageWay === 18) {
                theImage.source = ""
                variables.startAni = false
                theImage.x = - imageWidth  - 10
                theImage.y = variables.curWidH / 2 - imageHeight / 2
                imageNormal.imagePath = imageUrl
                theImage.source = imageNormal.imagePath
                variables.startAni = true
                theImage.x = variables.curWidW / 2 - imageWidth / 2
            }

        }
    }
    Connections {
        target: variables
        onWheelChangeChanged: {
            buriedPoint()
        }
        //story 19807
        onNeedRestoreOrigChanged: {
            imageBase.imageRestore(theImage)

        }
        //story 19807
    }

    PinchArea {

        id: pincharea

        anchors.fill: theImage

        scale: theImage.scale
        rotation: theImage.rotation
        enabled: true
        // the actual scale factor from a pinch event is the initial scale multiplied by Pinch.scale
        property real initialScale
        onPinchStarted: {
            initialScale = theImage.curScale
            buriedPoint()
        }
        onPinchUpdated: {
            if (isMoiveRunning()) {
                return
            }
            // 关闭动画
            variables.startAni = false
            variables.recordCurrentImageHasOperate = true
            if (!imageNeedCenterZoom) {
                imageBase.performZoom(theImage,theImage.mapFromItem(pincharea,pinch.center.x, pinch.center.y), undefined, false, false, true, (initialScale*pinch.scale)/theImage.curScale)
            } else {
                imageBase.performZoom(theImage,theImage.mapFromItem(pincharea, mainWindow.width/2, mainWindow.height/2), undefined, false, false, true,(initialScale*pinch.scale)/theImage.curScale)
                theImage.imagePostionReset()
            }
            // 开启动画
            variables.startAni = true
            imageBase.recordCUrrentPOsAndScaleInfo(theImage)
        }

        MouseArea {
            id: mousearea
            enabled: mouseareaEnabeled
            anchors.fill: parent

            drag.target: theImage

            hoverEnabled: false // 必须设置为false，否则，鼠标的坐标抓取的就不是全局的
            propagateComposedEvents:true //设置鼠标穿透，双击的事件需要传到底层

            onPressAndHold: {
                if (isMoiveRunning()) {
                    return
                }
                variables.mousePos = mousearea.mapToItem(bgimage, Qt.point(mouse.x, mouse.y))
            }

            onReleased: {
                if (isMoiveRunning()) {
                    return
                }
                if (variables.doubleRestore) {
                    variables.doubleRestore = false
                    return
                } else {
                    theImage.curX = theImage.x
                    theImage.curY = theImage.y
                    theImage.x = Qt.binding(function() { return theImage.curX })
                    theImage.y = Qt.binding(function() { return theImage.curY })
                }
                imageBase.controlImageInWid(theImage)
                imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
                imageBase.makeSureNavigatorNeedShow(theImage)
                if (variables.imageIsDrag) {
                    imageBase.recordCUrrentPOsAndScaleInfo(theImage)
                    variables.imageIsDrag = false
                }
            }
            onPositionChanged: {
                if (isMoiveRunning()) {
                    return
                }
                variables.imageIsDrag = true
                variables.recordCurrentImageHasOperate = true
                setDrogLimit()
                imageBase.imageActuralLeftUpPos(theImage,theImage.x,theImage.y)
                imageBase.makeSureNavigatorNeedShow(theImage)
                if (!variables.historyScale) {
                    imageBase.recordCUrrentPOsAndScaleInfo(theImage)
                }

            }

            Connections {
                target: variables
                onMousePosChanged: {
                    mousearea.cursorShape = Qt.ArrowCursor
                }
                onVisibleItemChanged: {
                    if (variables.visibleItem !== "") {
                        mousearea.cursorShape = Qt.ArrowCursor
                    } else {
                        mousearea.cursorShape = Qt.ArrowCursor
                    }
                }
            }
        }
    }

    Connections {
        target: container
        onZoomIn: {
            if (isMoiveRunning()) {
                return
            }
            //某些情况需要静止进行缩放
            if (needWheelReact == false) {
                return
            }
            variables.recordCurrentImageHasOperate = true

            if (wheelDelta != undefined && !imageNeedCenterZoom) {
                imageBase.performZoom(theImage,theImage.mapFromItem(bgimage, variables.mousePos.x, variables.mousePos.y), undefined, true, false, false)
            } else {
                imageBase.performZoom(theImage,theImage.mapFromItem(bgimage, mainWindow.width/2, mainWindow.height/2), undefined, true, false, false)
                theImage.imagePostionReset()
            }
            variables.operateImageWay = 1
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
            imageBase.makeSureNavigatorNeedShow(theImage)
            imageBase.recordCUrrentPOsAndScaleInfo(theImage)
        }

        onZoomOut: {
            if (isMoiveRunning()) {
                return
            }
            //某些情况需要静止进行缩放
            if (needWheelReact == false) {
                return
            }
            variables.recordCurrentImageHasOperate = true
            if (wheelDelta != undefined && !imageNeedCenterZoom) {
                imageBase.performZoom(theImage,theImage.mapFromItem(bgimage, variables.mousePos.x, variables.mousePos.y), undefined, false, false, false)
            } else {
                imageBase.performZoom(theImage,theImage.mapFromItem(bgimage, mainWindow.width/2, mainWindow.height/2), undefined, false, false, false)
                theImage.imagePostionReset()
            }
            variables.operateImageWay = 2
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
            imageBase.makeSureNavigatorNeedShow(theImage)
            imageBase.recordCUrrentPOsAndScaleInfo(theImage)
        }
        onRotate: {
            if (isMoiveRunning()) {
                return
            }
            theImage.rotateTo += deg
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        }

        onMirrorH: {
            if (isMoiveRunning()) {
                return
            }
            var old = theImage.mirror
            theImage.mirror = !old
            variables.startAni = false
            if (theImage.rotateTo / variables.angle90 % 2 != 0) {
                theImage.rotateTo += variables.angle180
            }
            variables.startAni = true
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        }
        onMirrorV: {
            if (isMoiveRunning()) {
                return
            }
            var old = theImage.mirror
            theImage.mirror = !old
            variables.startAni = false
            if (theImage.rotateTo / variables.angle90 % 2 == 0) {
                theImage.rotateTo += variables.angle180
            }
            variables.startAni = true
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        }
    }

    function setCurScale(zoomfactor) {
        if (isMoiveRunning()) {
            return
        }
        var curScale = theImage.curScale * zoomfactor

        if (curScale > 100 ) {
            theImage.curScale = 100
        } else if (curScale < 0.05) {
            theImage.curScale = 0.05
        }  else {
            theImage.curScale = curScale
        }
    }

    function operateImage(operateWay) {
        if (isMoiveRunning()) {
            return
        }
        variables.operateImageWay = operateWay
        if (operateWay === 1) {
            //放大
            container.zoomIn(undefined)
        } else if (operateWay === 2) {
            //缩小
            container.zoomOut(undefined)
        } else if (operateWay === 3) {
            variables.recordCurrentImageHasOperate = false
            variables.historyScale = 0
            variables.historyX = 0
            variables.historyY = 0
            //逆时针旋转
            container.rotate(-variables.angle90)
            theImage.imagePostionReset()
        } else if (operateWay === 4) {
            variables.recordCurrentImageHasOperate = false
            variables.historyScale = 0
            variables.historyX = 0
            variables.historyY = 0
            //顺时针旋转
            container.rotate(variables.angle90)
            theImage.imagePostionReset()
        } else if (operateWay === 5) {
            variables.recordCurrentImageHasOperate = false
            variables.historyScale = 0
            variables.historyX = 0
            variables.historyY = 0
            //水平
            container.mirrorH()
            theImage.imagePostionReset()
        } else if (operateWay === 6) {
            variables.recordCurrentImageHasOperate = false
            variables.historyScale = 0
            variables.historyX = 0
            variables.historyY = 0
            //垂直
            container.mirrorV()
            theImage.imagePostionReset()
        }
        //ocr和裁剪
        if (operateWay === 9 || operateWay === 10) {
            //禁止拖拽和缩放
            pincharea.enabled = false
            mousearea.enabled = false
            needWheelReact = false
        } else {
            //放开拖拽和缩放
            pincharea.enabled = true
            needWheelReact = true
        }
        //切图
        if (operateWay === 12 || operateWay === 17 || operateWay === 18) {
            variables.recordCurrentImageHasOperate = false
            variables.historyScale = 0
            variables.historyX = 0
            variables.historyY = 0
//            newImageLoaded = true
//            variables.startAni = false
//            theImage.imagePostionReset()
//            variables.startAni = true

            newImageLoaded = true
            variables.startAni = false
            theImage.rotateTo = 0
            theImage.mirror = false
            theImage.imagePostionReset()
            variables.startAni = true

        } else {
            newImageLoaded = false
        }
        //显示原图大小
        if (operateWay === 8) {
            theImage.curScale = 1
            theImage.imagePostionReset()
        }
    }
    //判断当前的图片加载模式，防止影响其他信号的接收
    function isMoiveRunning() {
        var currentImagePath  = "file://" + variables.currentPath
        if ((currentImagePath === theImage.source.toString() || theImage.source.toString() === "qrc:/res/res/loadgif.gif") && variables.currentImageIndex == theImage.index) {
            return false
        } else {
            return true
        }
    }

    //返回所需要相对于窗口的X和Y
    function getLeftUpPosAccordingWidX() {

        return imageBase.getLeftUpPosAccordingWidX(theImage)

    }
    function getLeftUpPosAccordingWidY() {

        return imageBase.getLeftUpPosAccordingWidY(theImage)
    }
    //返回所需要相对于窗口的在当前缩放比下的可视区域的高度和宽度
    function getVisibleSizeAccordingWidW() {

        return imageBase.getVisibleSizeAccordingWidW(theImage)

    }
    function getVisibleSizeAccordingWidH() {

        return imageBase.getVisibleSizeAccordingWidH(theImage)
    }
    //返回相对于原图的位置
    function getLeftUpPosAccordingImageX() {
        return  imageVisibleLeftUpX / theImage.curScale
    }
    function getLeftUpPosAccordingImageY() {
        return  imageVisibleLeftUpY / theImage.curScale
    }


    function operateNavigatorChangeImagePos(leftUpPoint) {
        imageBase.operateNavigatorChangeImagePos(leftUpPoint,theImage)
    }

    function setDrogLimit() {

        if(Math.abs(theImage.rotateTo % variables.angle360) == variables.angle90 || Math.abs(theImage.rotateTo % variables.angle360) == variables.angle270) {
            if (theImage.width * theImage.scale >= variables.curWidH) {
                if (theImage.height * theImage.scale >= variables.curWidW) {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAndYAxis
                    mouseDragType = 2
                } else {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.YAxis
                    mouseDragType = 1
                }
            } else if (theImage.height * theImage.scale >= variables.curWidW) {
                mousearea.enabled = true
                mousearea.drag.axis= Drag.XAxis
                mouseDragType = 0
            } else {
                mousearea.enabled = false
                mouseDragType = -1
            }
        } else {
            if (theImage.width * theImage.scale >= variables.curWidW) {
                if (theImage.height * theImage.scale >= variables.curWidH) {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAndYAxis
                    mouseDragType = 2
                } else {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAxis
                    mouseDragType = 0
                }
            } else if (theImage.height * theImage.scale >= variables.curWidH) {
                mousearea.enabled = true
                mousearea.drag.axis= Drag.YAxis
                mouseDragType = 1
            } else {
                mousearea.enabled = false
                mouseDragType = -1
            }
        }
        if(!variables.navigatorState) {
            mousearea.enabled = false
            mousearea.drag.axis = Drag.None
        } else {
            mousearea.enabled = true
        }
    }
    function adjustImagePostion(needRecord) {
        setDrogLimit()
        imageBase.controlImageInWid(theImage)
        if (needRecord === true) {
            variables.recordCurrentImageHasOperate = true
            imageBase.recordCUrrentPOsAndScaleInfo(theImage)
        }
    }
    function updatePosAccordingToWidSizeChange(needRecord) {
        imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        imageBase.makeSureNavigatorNeedShow(theImage)
        if (needRecord === true) {
            variables.recordCurrentImageHasOperate = true
            imageBase.recordCUrrentPOsAndScaleInfo(theImage)
        }
    }
}
