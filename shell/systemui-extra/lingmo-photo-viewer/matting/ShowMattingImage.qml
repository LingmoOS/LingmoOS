import QtQuick 2.12
import MattingInteractive 1.0

Item {
    objectName: "showMattingImageItem"
    id: mattingImageItem
    x:  parent.width/2 - width/2
    y:  parent.height/2 - height/2
    width: mattingContainer.width
    height: mattingContainer.height
    property bool needWheelReact: true
    property bool imageNeedCenterZoom: true
    property bool mouseareaEnabeled: false
    property bool newImageLoaded: false    //新图片加载
    property double imageVisibleLeftUpX: 0 //可视区域左上角的点在原图上的位置
    property double imageVisibleLeftUpY: 0
    property double imageLeftUpX: 0  //缩放后的图片的左上角相对于窗口的位置
    property double imageLeftUpY: 0
    property real defaultScale: 1.0
    property bool startAni: true
    property double realWidth : 0
    property double realHeight : 0
    property int mouseDragType: -1
    property bool isCutting: false
    property bool canOperate: true
    property double fixedScale: 0
    property int borderMargin: 3
    property color blurryColor: Qt.rgba(0,0,0,0.3)      //模糊色
    property color selectBorderColor: "#1D72F2"           //边框色
    signal sendScale(var scale)
    signal sendPos(var xPos,var yPos)
    signal changeOperateWay()
    signal cutFreeSize()
    signal closeChildWindow()

    Image {
        objectName: "mattingImage"
        id: mattingImage
        property real curX: mattingImageItem.width/2 - width/2
        property real curY: mattingImageItem.height/2 - height/2
        x: curX
        y: curY
        fillMode: Image.Pad
        focus: true
        clip: true
        cache: false
        asynchronous: true
        mirror: false
        property real curScale: 1
        scale: curScale
        property real rotateTo: 0
        rotation: 0
        property real curRotaion: 0
        property bool curMirror: false
        smooth: (width * scale > mattingvariables.curWidW || height > mattingvariables.curWidH)
        //保存图像在缩放或变换时是否使用mipmap过滤。与平滑相比，缩小比例时，mipmap过滤提供了更好的视觉质量，但它可能会以性能为代价（无论是在初始化图像时还是在渲染期间）。
        mipmap: (scale < defaultScale || (scale < 0.8 && defaultScale < 0.8)) && (width > mattingvariables.curWidW || height > mattingvariables.curWidH)

        MouseArea{
            id: matmouse
            anchors.fill: parent
            hoverEnabled: false
            drag.target: isCutting ? null : mattingImage
            property int startX: 0
            property int startY: 0
            property int endX: 0
            property int endY: 0

            onPressed: function (mouse){
                if (canOperate && (fixedScale==0))
                {
                    var beginpoint = mapToItem(mattingBgImage, mouse.x, mouse.y);
                    startX = beginpoint.x;
                    startY = beginpoint.y;
                    selectionRect.width = 0;
                    selectionRect.height = 0;
                    selectionRect.visible = isCutting;
                    cutFreeSize();
                }
            }
            onPositionChanged: function (mouse){
                if (pressed && canOperate && (fixedScale==0)) {
                    var endpoint = mapToItem(mattingBgImage, mouse.x, mouse.y);
                    endX = endpoint.x;
                    endY = endpoint.y;
                    mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)

                    if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                       Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                    {
                        if ((endX < imageLeftUpX + mattingImage.height * mattingImage.curScale) &&
                            (endY < imageLeftUpY + mattingImage.width * mattingImage.curScale) &&
                            (endX>imageLeftUpX) && (endY>imageLeftUpY))
                        {
                            selectionRect.width = Math.abs(endX - startX);
                            selectionRect.height = Math.abs(endY - startY);
                            selectionRect.x = Math.min(startX, endX);
                            selectionRect.y = Math.min(startY, endY);
                            updateGrid(true);
                        }
                    }
                    else
                    {
                        if ((endX < imageLeftUpX + mattingImage.width * mattingImage.curScale) &&
                            (endY < imageLeftUpY + mattingImage.height * mattingImage.curScale) &&
                            (endX>imageLeftUpX) && (endY>imageLeftUpY))
                        {
                            selectionRect.width = Math.abs(endX - startX);
                            selectionRect.height = Math.abs(endY - startY);
                            selectionRect.x = Math.min(startX, endX);
                            selectionRect.y = Math.min(startY, endY);
                            updateGrid(true);
                        }
                    }
                }
            }
            onReleased: function (mouse){
                if (canOperate && (fixedScale==0)) {
                    selectionRect.updateStartAndEndPoint();
                    updateGrid(false);
                }
            }

            onClicked: {
                closeChildWindow()
            }
        }

        onRotateToChanged: {
            rotation = rotateTo
        }

        onSourceSizeChanged: {
            curX = mattingImageItem.width/2 - width/2
            curY = mattingImageItem.height/2 - height/2
            mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
        }

        onSourceChanged: {
            texture.visible = true
            texture.hideSource = true
        }

        onRotationChanged: {
            if(!rotani.running) {
                rotateTo = rotation
            }

            texture.visible = false
            texture.hideSource = false
        }

        onScaleChanged: {
            mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)

            texture.visible = false
            texture.hideSource = false
        }

        Behavior on x {
            PropertyAnimation {
                id: xani
                duration: startAni ? mattingvariables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }

        Behavior on y {
            PropertyAnimation {
                id: yani
                duration: startAni ? mattingvariables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }

        Behavior on scale {
            PropertyAnimation {
                id: scaleani
                duration: startAni ? mattingvariables.imageAniDuration : 0
                onRunningChanged: {
                    if (!scaleani.running) {
                        mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
                    }
                }
                easing.type: Easing.OutCubic
            }
        }

        Behavior on rotation {
            PropertyAnimation {
                id: rotani
                duration: startAni ? mattingvariables.imageAniDuration : 0
                onRunningChanged: {
                    if (!rotani.running) {
                        mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
                    }
                }
                easing.type: Easing.OutCubic
            }
        }

        onXChanged: {
            setDrogLimit()
        }

        onYChanged: {
            setDrogLimit()
        }

        onStatusChanged: {
            if (mattingImage.status == Image.Ready) {
                if (newImageLoaded) {
                    mattingImage.mattingImagePostionReset()
                }
            }
        }

        //重设位置
        function mattingImagePostionReset() {
            mattingbasefunction.resetImagePostion(mattingImage)
        }
        //设置图片缩放比
        function sendMattingImageScale() {
            return mattingbasefunction.getImageScale(mattingImage)
        }
        function changeMattingScale(scaleValue) {
            mattingbasefunction.setImageScaleWithImageUpdate(mattingImage,scaleValue)
        }
        function changeMattingImage(imageUrl,imageWidth,imageHeight) {
            realWidth = imageWidth
            realHeight = imageHeight
            mattingImage.source = ""
            startAni = false
            mattingImage.x = mattingvariables.curWidW / 2 - imageWidth / 2
            mattingImage.y = mattingvariables.curWidH / 2 - imageHeight / 2
            mattingImage.source = imageUrl
            startAni = true
        }
        function getOperateImageWay() {
            return mattingvariables.operateImageWay
        }
    }

    PinchArea {
        id: pincharea
        anchors.fill: mattingImage
        scale: mattingImage.scale
        rotation: mattingImage.rotation
        enabled: true

        property real initialScale
        onPinchStarted: {
            initialScale = mattingImage.curScale
        }
        onPinchUpdated: {
            // 关闭动画
            startAni = false
            if (!imageNeedCenterZoom) {
                mattingbasefunction.performZoom(mattingImage,mattingImage.mapFromItem(pincharea,pinch.center.x, pinch.center.y), undefined, false, false, true, (initialScale*pinch.scale)/mattingImage.curScale)
            } else {
                mattingbasefunction.performZoom(mattingImage,mattingImage.mapFromItem(pincharea, mattingWindow.width/2, mattingWindow.height/2), undefined, false, false, true,(initialScale*pinch.scale)/mattingImage.curScale)
                mattingImage.mattingImagePostionReset()
            }
            // 开启动画
            startAni = true
        }

        MouseArea {
            id: mousearea
            enabled: mouseareaEnabeled
            anchors.fill: parent
            drag.target: mattingImage
            hoverEnabled: false // 必须设置为false，否则，鼠标的坐标抓取的就不是全局的
            propagateComposedEvents: true
            onPressAndHold: {
                mattingvariables.mousePos = mousearea.mapToItem(mattingBgImage, Qt.point(mouse.x, mouse.y))
            }
            onReleased: {
                mattingImage.curX = mattingImage.x
                mattingImage.curY = mattingImage.y
                mattingImage.x = Qt.binding(function() { return mattingImage.curX })
                mattingImage.y = Qt.binding(function() { return mattingImage.curY })
                mattingbasefunction.controlImageInWid(mattingImage)
                mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
                mattingbasefunction.makeSureImageOverWid(mattingImage)
            }
            onPositionChanged: {
                setDrogLimit()
                mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.x ,mattingImage.y)
                mattingbasefunction.makeSureImageOverWid(mattingImage)
            }
        }
    }

    ShaderEffectSource {
        id: texture
        anchors.fill: mattingImage
        sourceItem: mattingImage
        scale: mattingImage.scale
        live: mattingImage.status === Image.Ready
        hideSource: true
    }

    Connections{
        target: MattingImage
        onCallQmlRefeshImg:{
            mattingImage.source = "image://MattingImg/" + Math.random()
        }
    }

    Connections {
        target: mattingContainer
        onZoomOut: {
            if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
               Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
            {
                if ((selectionRect.width + 2 >= mattingImage.height*mattingImage.curScale) ||
                        (selectionRect.height + 2 >= mattingImage.width*mattingImage.curScale))
                {
                    mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
                    if (selectionRect.x < imageLeftUpX)
                    {
                        selectionRect.x = imageLeftUpX
                    }
                    if (selectionRect.y < imageLeftUpY)
                    {
                        selectionRect.y = imageLeftUpY
                    }
                    if (selectionRect.width > mattingImage.height*mattingImage.curScale)
                       selectionRect.width = mattingImage.height*mattingImage.curScale-1
                    if (selectionRect.height > mattingImage.width*mattingImage.curScale)
                       selectionRect.height = mattingImage.width*mattingImage.curScale-1
                    selectionRect.updateStartAndEndPoint()
                    return
                }
            }
            else
            {
                if ((selectionRect.width + 2 >= mattingImage.width*mattingImage.curScale) ||
                        (selectionRect.height + 2 >= mattingImage.height*mattingImage.curScale))
                {
                    mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
                    if (selectionRect.x < imageLeftUpX)
                    {
                        selectionRect.x = imageLeftUpX
                    }
                    if (selectionRect.y < imageLeftUpY)
                    {
                        selectionRect.y = imageLeftUpY
                    }

                    if (selectionRect.width > mattingImage.width*mattingImage.curScale)
                       selectionRect.width = mattingImage.width*mattingImage.curScale-1
                    if (selectionRect.height > mattingImage.height*mattingImage.curScale)
                       selectionRect.height = mattingImage.height*mattingImage.curScale-1
                    selectionRect.updateStartAndEndPoint()
                    return
                }
            }

            if (wheelDelta != undefined && !imageNeedCenterZoom) {
                mattingbasefunction.performZoom(mattingImage,mattingImage.mapFromItem(mattingBgImage, mattingvariables.mousePos.x, mattingvariables.mousePos.y), undefined, false, false, false)
            } else {
                mattingbasefunction.performZoom(mattingImage,mattingImage.mapFromItem(mattingBgImage, mattingWindow.width/2, mattingWindow.height/2), undefined, false, false, false)
                mattingImage.mattingImagePostionReset()
            }
            mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
            mattingbasefunction.makeSureImageOverWid(mattingImage)
            mattingvariables.operateImageWay = 2
            changeOperateWay()
            mattingvariables.imageZoomSpeed = 15

            if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) === mattingvariables.angle90 ||
               Math.abs(mattingImage.rotateTo % mattingvariables.angle360) === mattingvariables.angle270)
            {
                if (selectionRect.x < imageLeftUpX)
                {
                    selectionRect.x = imageLeftUpX
                }
                else if (selectionRect.x+selectionRect.width>imageLeftUpX+mattingImage.height*mattingImage.curScale)
                {
                    selectionRect.x = imageLeftUpX+mattingImage.height*mattingImage.curScale-selectionRect.width
                }

                if (selectionRect.y < imageLeftUpY)
                {
                    selectionRect.y = imageLeftUpY
                }
                else if (selectionRect.y+selectionRect.height>imageLeftUpY+mattingImage.width*mattingImage.curScale)
                {
                    selectionRect.y = imageLeftUpY+mattingImage.width*mattingImage.curScale-selectionRect.height
                }
            }
            else
            {
                if (selectionRect.x < imageLeftUpX)
                {
                    selectionRect.x = imageLeftUpX
                }
                else if (selectionRect.x+selectionRect.width>imageLeftUpX+mattingImage.width*mattingImage.curScale)
                {
                    selectionRect.x = imageLeftUpX+mattingImage.width*mattingImage.curScale-selectionRect.width
                }

                if (selectionRect.y < imageLeftUpY)
                {
                    selectionRect.y = imageLeftUpY
                }
                else if (selectionRect.y+selectionRect.height>imageLeftUpY+mattingImage.height*mattingImage.curScale)
                {
                    selectionRect.y = imageLeftUpY+mattingImage.height*mattingImage.curScale-selectionRect.height
                }
            }
        }
        onZoomIn: {
            if (wheelDelta != undefined && !imageNeedCenterZoom) {
                mattingbasefunction.performZoom(mattingImage,mattingImage.mapFromItem(mattingBgImage, mattingvariables.mousePos.x, mattingvariables.mousePos.y), undefined, true, false, false)
            } else {
                mattingbasefunction.performZoom(mattingImage,mattingImage.mapFromItem(mattingBgImage, mattingWindow.width/2, mattingWindow.height/2), undefined, true, false, false)
                mattingImage.mattingImagePostionReset()
            }
            mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
            mattingbasefunction.makeSureImageOverWid(mattingImage)
            mattingvariables.operateImageWay = 1
            changeOperateWay()
            mattingvariables.imageZoomSpeed = 15
        }
        onRotate: {
            mattingImage.rotateTo += deg
            mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
        }
        onMirrorH: {
            var old = mattingImage.mirror
            mattingImage.mirror = !old
            startAni = false
            if (mattingImage.rotateTo / mattingvariables.angle90 % 2 != 0) {
                mattingImage.rotateTo += mattingvariables.angle180
            }
            startAni = true
            mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
        }
        onMirrorV: {
            var old = mattingImage.mirror
            mattingImage.mirror = !old
            startAni = false
            if (mattingImage.rotateTo / mattingvariables.angle90 % 2 == 0) {
                mattingImage.rotateTo += mattingvariables.angle180
            }
            startAni = true
            mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
        }
    }

    Connections {
        target: mattingvariables
        onOperateImageWayChanged: {
            if (mattingvariables.operateImageWay === 3) {
                //逆时针旋转
                startAni = true
                mattingContainer.rotate(-mattingvariables.angle90)
                mattingImage.mattingImagePostionReset()
                startAni = false
            } else if (mattingvariables.operateImageWay === 4) {
                //顺时针旋转
                startAni = true
                mattingContainer.rotate(mattingvariables.angle90)
                mattingImage.mattingImagePostionReset()
                startAni = false
            } else if (mattingvariables.operateImageWay === 5) {
                //水平
                mattingContainer.mirrorH()
                mattingImage.mattingImagePostionReset()
            } else if (mattingvariables.operateImageWay === 6) {
                //垂直
                mattingContainer.mirrorV()
                mattingImage.mattingImagePostionReset()
            }
        }
    }


    //四个阴影区域，笼罩未选择区域
    //一个选择区域
    Rectangle{
        id: topRect
        anchors{
            top: parent.top
            left: parent.left
        }
        width: parent.width
        height: selectionRect.y
        color: blurryColor
        visible: isCutting
    }

    Rectangle{
        id: leftRect
        anchors{
            top: topRect.bottom
            left: parent.left
        }
        width: selectionRect.x
        height: parent.height - topRect.height - bottomRect.height
        color: blurryColor
        visible: isCutting
    }

    Rectangle{
        id: rightRect
        anchors{
            top: topRect.bottom
            right: parent.right
        }
        width: parent.width - leftRect.width - selectionRect.width
        height: leftRect.height
        color: blurryColor
        visible: isCutting
    }

    Rectangle{
        id: bottomRect
        anchors{
            bottom: parent.bottom
            left: parent.left
        }
        width: parent.width
        height: parent.height - topRect.height - selectionRect.height
        color: blurryColor
        visible: isCutting
    }

    Grid {
        id: grid
        anchors.fill: selectionRect
        columns: 3
        rows: 3

        Repeater {
            id:repeater
            model: grid.columns*grid.rows
            Rectangle {
                id: repeaterRect
                width: grid.width/grid.columns
                height: grid.height/grid.rows
                color: "transparent"
                border.color: "#FAFAFA"
                border.width: 1
                opacity: 0.7
            }
        }
    }

    Rectangle {
        id: selectionRect
        property point startPoint
        property point endPoint

        function updateStartAndEndPoint() {
            startPoint = Qt.point(x, y);
            endPoint = Qt.point(x + width, y + height);
        }

        visible: false
        border.color: selectBorderColor
        border.width: 1
        color: "transparent"

        MouseArea {
            id: dragItem
            anchors.fill: parent
            anchors.margins: 12 * 2
            cursorShape: Qt.SizeAllCursor
            drag.target: parent
            drag.minimumX: imageLeftUpX
            drag.maximumX: imageLeftUpX+(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) === mattingvariables.angle90 ||
                                         Math.abs(mattingImage.rotateTo % mattingvariables.angle360) === mattingvariables.angle270 ? mattingImage.height : mattingImage.width)*mattingImage.curScale-parent.width
            drag.minimumY: imageLeftUpY
            drag.maximumY: imageLeftUpY+(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) === mattingvariables.angle90 ||
                                         Math.abs(mattingImage.rotateTo % mattingvariables.angle360) === mattingvariables.angle270 ? mattingImage.width : mattingImage.height)*mattingImage.curScale-parent.height
            onPositionChanged: {
                selectionRect.updateStartAndEndPoint();
            }
        }

        Item{
            id: resizeBorderItem
            anchors.centerIn: parent
            width: parent.width + borderMargin * 1
            height: parent.height + borderMargin * 1
        }

        //LeftTop
        CusDragRect{
            id: dragLeftTop
            anchors{
                left: resizeBorderItem.left
                top: resizeBorderItem.top
            }
            visible: selectionRect.visible && (selectionRect.width > 5)
            callBackFunc : selectionRect.updateStartAndEndPoint
            posType: posLeftTop
            imgType: imgLeftTop

            onSigPosChanged: function(mousePoint){
                var point = mousePoint;
                var pos_x = Math.min(point.x, selectionRect.endPoint.x);
                var pos_y = Math.min(point.y, selectionRect.endPoint.y);
                var nWidth = Math.max(selectionRect.endPoint.x, point.x)  - pos_x;
                var nHeight = 0;
                if (fixedScale > 0)
                {
                    nHeight = nWidth/fixedScale
                }
                else if (fixedScale < 0)
                {
                    nHeight = nWidth*fixedScale
                }
                else
                {
                    nHeight = Math.max(selectionRect.endPoint.y, point.y) - pos_y;
                }

                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        selectionRect.x = pos_x
                        selectionRect.y = pos_y
                        selectionRect.width = nWidth
                        selectionRect.height = nHeight
                    }
                }
                else
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        selectionRect.x = pos_x
                        selectionRect.y = pos_y
                        selectionRect.width = nWidth
                        selectionRect.height = nHeight
                    }
                }
            }
        }

        //LeftBottom
        CusDragRect{
            id: dragLeftBottom
            anchors{
                left: resizeBorderItem.left
                bottom: resizeBorderItem.bottom
            }
            visible: selectionRect.visible && (selectionRect.height > 30)
            callBackFunc : selectionRect.updateStartAndEndPoint
            posType: posLeftBottom
            imgType: imgLeftBottom

            onSigPosChanged: function(mousePoint){
                var point = mousePoint;
                var pos_x = Math.min(point.x, selectionRect.endPoint.x);
                var pos_y = Math.min(point.y, selectionRect.startPoint.y);
                var nWidth = Math.max(selectionRect.endPoint.x, point.x)  - pos_x;
                var nHeight = 0;
                if (fixedScale > 0)
                {
                    nHeight = nWidth/fixedScale
                }
                else if (fixedScale < 0)
                {
                    nHeight = nWidth*fixedScale
                }
                else
                {
                    nHeight = Math.max(selectionRect.startPoint.y, point.y) - pos_y;
                }

                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        selectionRect.x = pos_x;
                        selectionRect.width = nWidth;
                        selectionRect.height = nHeight;
                    }
                }
                else
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        selectionRect.x = pos_x;
                        selectionRect.width = nWidth;
                        selectionRect.height = nHeight;
                    }
                }
            }
        }

        //RightTop
        CusDragRect{
            id: dragRightTop
            anchors{
                right: resizeBorderItem.right
                top: resizeBorderItem.top
            }
            visible: selectionRect.visible && (selectionRect.width > 30)
            callBackFunc : selectionRect.updateStartAndEndPoint
            posType: posRightTop
            imgType: imgRightTop

            onSigPosChanged: function(mousePoint){
                var point = mousePoint;
                var pos_x = Math.min(point.x, selectionRect.startPoint.x);
                var pos_y = Math.min(point.y, selectionRect.endPoint.y);
                var nWidth = Math.max(selectionRect.startPoint.x, point.x)  - selectionRect.x;
                var nHeight = 0;
                if (fixedScale > 0)
                {
                    nHeight = nWidth/fixedScale
                }
                else if (fixedScale < 0)
                {
                    nHeight = nWidth*fixedScale
                }
                else
                {
                    nHeight = Math.max(selectionRect.endPoint.y, point.y) - pos_y;
                }

                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        selectionRect.y = pos_y
                        selectionRect.width = nWidth
                        selectionRect.height = nHeight
                    }
                }
                else
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        selectionRect.y = pos_y
                        selectionRect.width = nWidth
                        selectionRect.height = nHeight
                    }
                }
            }
        }

        //RightBottom
        CusDragRect{
            id: dragRightBottom
            anchors{
                right: resizeBorderItem.right
                bottom: resizeBorderItem.bottom
            }
            visible: selectionRect.visible && (selectionRect.width > 30)
            callBackFunc : selectionRect.updateStartAndEndPoint
            posType: posRightBottom
            imgType: imgRightBottom

            onSigPosChanged: function(mousePoint){
                var point = mousePoint;
                var pos_x = Math.min(point.x, selectionRect.startPoint.x);
                var pos_y = Math.min(point.y, selectionRect.startPoint.y);
                var nWidth = Math.max(selectionRect.startPoint.x, point.x)  - selectionRect.x;
                var nHeight = 0;
                if (fixedScale > 0)
                {
                    nHeight = nWidth/fixedScale
                }
                else if (fixedScale < 0)
                {
                    nHeight = nWidth*fixedScale
                }
                else
                {
                    nHeight = Math.max(selectionRect.startPoint.y, point.y) - selectionRect.y;
                }

                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        selectionRect.width = nWidth
                        selectionRect.height = nHeight
                    }
                }
                else
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        selectionRect.width = nWidth
                        selectionRect.height = nHeight
                    }
                }
            }
        }

        //Top
        CusDragRect{
            id: dragTop
            anchors{
                top: resizeBorderItem.top
                horizontalCenter: resizeBorderItem.horizontalCenter
            }
            visible: selectionRect.visible && (selectionRect.width > 20)
            callBackFunc : selectionRect.updateStartAndEndPoint
            posType: posTop
            imgType: imgTop
            verticalAlignType: Image.AlignTop

            onSigPosChanged: function(mousePoint){
                var point = mousePoint;
                var pos_x = Math.min(point.x, selectionRect.startPoint.x);
                var pos_y = Math.min(point.y, selectionRect.endPoint.y);
                var nHeight = Math.max(selectionRect.endPoint.y, point.y) - pos_y;
                var nWidth = 0;
                if (fixedScale > 0)
                {
                    nWidth = nHeight*fixedScale
                }
                else if (fixedScale < 0)
                {
                    nWidth = nHeight/fixedScale
                }

                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        selectionRect.y = pos_y
                        selectionRect.height = nHeight
                        if (fixedScale != 0)
                        {
                            selectionRect.width = nWidth
                        }

                    }
                }
                else
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        selectionRect.y = pos_y
                        selectionRect.height = nHeight
                        if (fixedScale != 0)
                        {
                            selectionRect.width = nWidth
                        }
                    }
                }
            }
        }

        //Bottom
        CusDragRect{
            id: dragBottom
            anchors{
                bottom: resizeBorderItem.bottom
                horizontalCenter: resizeBorderItem.horizontalCenter
            }
            visible: selectionRect.visible && (selectionRect.width > 20)
            callBackFunc : selectionRect.updateStartAndEndPoint
            posType: posBottom
            imgType: imgBottom
            verticalAlignType: Image.AlignBottom

            onSigPosChanged: function(mousePoint){
                var point = mousePoint;
                var pos_x = Math.min(point.x, selectionRect.startPoint.x);
                var pos_y = Math.min(point.y, selectionRect.startPoint.y);
                var nHeight = Math.max(selectionRect.startPoint.y, point.y) - selectionRect.y;
                var nWidth = 0;
                if (fixedScale > 0)
                {
                    nWidth = nHeight*fixedScale
                }
                else if (fixedScale < 0)
                {
                    nWidth = nHeight/fixedScale
                }

                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        if (fixedScale != 0)
                        {
                            selectionRect.width = nWidth
                        }
                        selectionRect.height = nHeight
                    }
                }
                else
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        if (fixedScale != 0)
                        {
                            selectionRect.width = nWidth
                        }
                        selectionRect.height = nHeight
                    }
                }
            }
        }

        //Left
        CusDragRect{
            id: dragLeft
            anchors{
                left: resizeBorderItem.left
                verticalCenter: resizeBorderItem.verticalCenter
            }
            visible: selectionRect.visible && (selectionRect.height > 20)
            callBackFunc : selectionRect.updateStartAndEndPoint
            posType: posLeft
            imgType: imgLeft
            horizontalAlignType: Image.AlignLeft

            onSigPosChanged: function(mousePoint){
                var point = mousePoint;
                var pos_x = Math.min(point.x, selectionRect.endPoint.x);
                var pos_y = Math.min(point.y, selectionRect.startPoint.y);
                var nWidth = Math.max(selectionRect.endPoint.x, point.x)  - pos_x;
                var nHeight = 0;
                if (fixedScale > 0)
                {
                    nHeight = nWidth/fixedScale
                }
                else if (fixedScale < 0)
                {
                    nHeight = nWidth*fixedScale
                }

                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        selectionRect.x = pos_x
                        selectionRect.width = nWidth
                        if (fixedScale != 0)
                        {
                            selectionRect.height = nHeight
                        }
                    }
                }
                else
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        selectionRect.x = pos_x
                        selectionRect.width = nWidth
                        if (fixedScale != 0)
                        {
                            selectionRect.height = nHeight
                        }
                    }
                }
            }
        }

        //Right
        CusDragRect{
            id: dragRight
            anchors{
                right: resizeBorderItem.right
                verticalCenter: resizeBorderItem.verticalCenter
            }
            visible: selectionRect.visible && (selectionRect.height > 20)
            callBackFunc : selectionRect.updateStartAndEndPoint
            posType: posRight
            imgType: imgRight
            horizontalAlignType: Image.AlignRight

            onSigPosChanged: function(mousePoint){
                var point = mousePoint;
                var pos_x = Math.min(point.x, selectionRect.startPoint.x);
                var pos_y = Math.min(point.y, selectionRect.startPoint.y);
                var nWidth = Math.max(selectionRect.startPoint.x, point.x)  - selectionRect.x;
                var nHeight = 0;
                if (fixedScale > 0)
                {
                    nHeight = nWidth/fixedScale
                }
                else if (fixedScale < 0)
                {
                    nHeight = nWidth*fixedScale
                }

                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        selectionRect.width = nWidth
                        if (fixedScale != 0)
                        {
                            selectionRect.height = nHeight
                        }
                    }
                }
                else
                {
                    if ((pos_x >= imageLeftUpX) && (pos_y >= imageLeftUpY) &&
                            (pos_x+nWidth <= imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos_y+nHeight <= imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        selectionRect.width = nWidth
                        if (fixedScale != 0)
                        {
                            selectionRect.height = nHeight
                        }
                    }
                }
            }
        }
    }

    function startCut() {
        // 242366 在个别机器上，mattingImagePostionReset执行图片居中无效，在这里刷新下x和y的值
        if (mattingImage.x !== mattingImage.curX)
        {
            mattingImage.curX = mattingImage.curX + 0.01
            mattingImage.curX = mattingImage.curX - 0.01
        }
        if (mattingImage.y !== mattingImage.curY)
        {
            mattingImage.curY = mattingImage.curY + 0.01
            mattingImage.curY = mattingImage.curY - 0.01
        }
        mattingImage.mattingImagePostionReset()

        isCutting = true;
        canOperate = true;
        selectionRect.visible = false;
        selectionRect.width = 0;
        selectionRect.height = 0;
    }

    function quitCut() {
        isCutting = false;
        selectionRect.visible = false;
        selectionRect.width = 0;
        selectionRect.height = 0;
    }

    function setCutSize(width, height, x, y, canOper, fixedSca) {
        selectionRect.width = width;
        selectionRect.height = height;
        selectionRect.x = x;
        selectionRect.y = y;
        selectionRect.visible = true;
        selectionRect.updateStartAndEndPoint();

        canOperate = canOper;
        fixedScale = fixedSca
        isCutting = true;
        grid.visible = false;
    }

    function setCutPos() {
        if (selectionRect.visible)
        {
            var fromImg_x =  selectionRect.x - imageLeftUpX
            var fromImg_y =  selectionRect.y - imageLeftUpY
            mattingImage.mattingImagePostionReset()
            mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
            selectionRect.x = fromImg_x + imageLeftUpX;
            selectionRect.y = fromImg_y + imageLeftUpY;
            selectionRect.updateStartAndEndPoint();
        }
    }

    function getCutInfoX(){
        return selectionRect.x - imageLeftUpX;
    }

    function getCutInfoY(){
        return selectionRect.y - imageLeftUpY;
    }

    function getCutInfoW(){
        return selectionRect.width;
    }

    function getCutInfoH(){
        return selectionRect.height;
    }

    function checkCutSize(){
        if ((selectionRect.width == 0) || (selectionRect.height == 0))
        {
            return false;
        }

        mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)

        var beginX = getCutInfoX()
        var beginY = getCutInfoY()
        if ((beginX < 0) || (beginY < 0))
        {
            return false;
        }

        var visibleW
        var visibleH
        if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
           Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270) {
            visibleH = mattingImage.width * mattingImage.curScale
            visibleW =  mattingImage.height * mattingImage.curScale
        } else {
            visibleW = mattingImage.width * mattingImage.curScale
            visibleH =  mattingImage.height * mattingImage.curScale
        }

        var endX = beginX + selectionRect.width
        var endY = beginY + selectionRect.height
        if ((endX > visibleW) || (endY > visibleH)) {
            return false;
        }

        return true;
    }

    function updateGrid(visible){
        grid.width = selectionRect.width
        grid.height = selectionRect.height
        grid.update()
        grid.visible = visible && selectionRect.visible && canOperate;
    }

    function getRotateTo(){
        return mattingImage.rotateTo;
    }
    
    function mattingClear() {
        startAni = false
        mattingImage.rotateTo = 0
        mattingImage.mirror = false
        mattingImage.mattingImagePostionReset()
    }

    function mattingOperateImage(operateWay) {
        mattingvariables.operateWay = operateWay
        if (operateWay === 1) {
            //放大
            mattingContainer.zoomIn(undefined)
        } else if (operateWay === 2) {
            //缩小
            mattingContainer.zoomOut(undefined)
        }
        if (operateWay === 12) {
            startAni = false
            mattingImage.rotateTo = 0
            mattingImage.mirror = false
            newImageLoaded = true
            mattingImage.mattingImagePostionReset()
        } else {
            newImageLoaded = false
        }
        if (operateWay === 8) {
            mattingImage.curScale = 1
            mattingImage.mattingImagePostionReset()
        }
    }
    function comparePressedShowImage() {
        mattingImage.curRotaion = mattingImage.rotation
        mattingImage.curMirror = mattingImage.mirror
    }
    function compareReleasedShowImage() {
        mattingImage.rotateTo = mattingImage.curRotaion
        mattingImage.mirror = mattingImage.curMirror
        mattingImage.mattingImagePostionReset()
    }
    //拖拽图片限制
    function setDrogLimit() {
        if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 || Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270) {
            if (mattingImage.width * mattingImage.scale >= mattingvariables.curWidH) {
                if (mattingImage.height * mattingImage.scale >= mattingvariables.curWidW) {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAndYAxis
                    mouseDragType = 2
                } else {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.YAxis
                    mouseDragType = 1
                }
            } else if (mattingImage.height * mattingImage.scale >= mattingvariables.curWidW) {
                mousearea.enabled = true
                mousearea.drag.axis= Drag.XAxis
                mouseDragType = 0
            } else {
                mousearea.enabled = false
                mouseDragType = -1
            }
        } else {
            if (mattingImage.width * mattingImage.scale >= mattingvariables.curWidW) {
                if (mattingImage.height * mattingImage.scale >= mattingvariables.curWidH) {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAndYAxis
                    mouseDragType = 2
                } else {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAxis
                    mouseDragType = 0
                }
            } else if (mattingImage.height * mattingImage.scale >= mattingvariables.curWidH) {
                mousearea.enabled = true
                mousearea.drag.axis= Drag.YAxis
                mouseDragType = 1
            } else {
                mousearea.enabled = false
                mouseDragType = -1
            }
        }
        if(!mattingvariables.isOverWidSize) {
            mousearea.enabled = false
        } else {
            mousearea.enabled = true
        }
    }

    function adjustMattingImagePostion() {
        setDrogLimit()
        mattingbasefunction.controlImageInWid(mattingImage)
    }
    function updateMattingPosAccordingToWidSizeChange() {
        mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
        mattingbasefunction.makeSureImageOverWid(mattingImage)
        mattingImage.mattingImagePostionReset()
    }
}
