import QtQuick 2.12
import ScannerInteractive 1.0

Item {
    objectName: "showScannerImageItem"
    id: scannerImageItem
    x:  parent.width/2 - width/2
    y:  parent.height/2 - height/2
    width: scannerContainer.width
    height: scannerContainer.height

    property real defaultScale: 1.0
    property bool needWheelReact: true
    property bool mouseareaEnabeled: false
    property bool imageNeedCenterZoom: true //中心缩放
    property bool newImageLoaded: false     //图片加载
    property bool startAni: true
    property double curMouseX: 0
    property double curMouseY: 0
    property double realWidth : 0
    property double realHeight : 0
    property double imageLeftUpX: 0  //缩放后的图片的左上角相对于窗口的位置
    property double imageLeftUpY: 0
    //裁剪
    property bool isCutting: false
    property bool canOperate: true
    property int borderMargin: 3
    property color blurryColor: Qt.rgba(0,0,0,0.3)      //模糊色
    property color selectBorderColor: "#1D72F2"           //边框色

    property double imageVisibleLeftUpX: 0 //可视区域左上角的点在原图上的位置
    property double imageVisibleLeftUpY: 0

    //自定义信号
    signal sendScale(var scale)
    signal sendPos(var xPos,var yPos)
    signal changeOperateWay()
    signal sigWheelZoom()

    Image {
        objectName: "scannerImage"
        id: scannerImage
        x: curX
        y: curY
        fillMode: Image.Pad //原始图像，不会被拉伸或裁剪
        clip: true          //超过窗口部分裁剪
        scale: curScale
        mirror: false       
        rotation: 0
        //数据的实时性
        cache: false
        asynchronous: true

        property real curX: scannerImageItem.width/2 - width/2
        property real curY: scannerImageItem.height/2 - height/2
        property real curScale: 1
        property real rotateTo: 0
        property real curRotaion: 0
        property bool curMirror: false

        //平滑缩放
        smooth: (width * scale > scannerVariables.curWidW || height > scannerVariables.curWidH)
        mipmap: (scale < defaultScale || (scale < 0.8 && defaultScale < 0.8)) && (width > scannerVariables.curWidW || height > scannerVariables.curWidH)

        //裁剪
        MouseArea{
            id: scanmouse
            anchors.fill: parent
            drag.target: isCutting ? null : scannerImage
            property int startX: 0
            property int startY: 0
            property int endX: 0
            property int endY: 0

            onPressed: function (mouse){
                if (canOperate)
                {
                    var beginpoint = mapToItem(scannerBgImage, mouse.x, mouse.y);
                    startX = beginpoint.x;
                    startY = beginpoint.y;
                    selectionRect.width = 0;
                    selectionRect.height = 0;
                    selectionRect.visible = isCutting;
                }
            }
            onPositionChanged: function (mouse){
                if (pressed && canOperate) {
                    var endpoint = mapToItem(scannerBgImage, mouse.x, mouse.y);
                    endX = endpoint.x;
                    endY = endpoint.y;
                    scannerBasefunction.imageActuralLeftUpPos(scannerImage,scannerImage.curX,scannerImage.curY)

                    if(Math.abs(scannerImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 ||
                       Math.abs(scannerImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270)
                    {
                        if ((endX < imageLeftUpX + scannerImage.height * scannerImage.curScale) &&
                            (endY < imageLeftUpY + scannerImage.width * scannerImage.curScale) &&
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
                        if ((endX < imageLeftUpX + scannerImage.width * scannerImage.curScale) &&
                            (endY < imageLeftUpY + scannerImage.height * scannerImage.curScale) &&
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
                if (canOperate) {
                    selectionRect.updateStartAndEndPoint();
                }
            }
        }

        onRotateToChanged: {
            rotation = rotateTo
            scannerBasefunction.imageActuralLeftUpPos(scannerImage,scannerImage.curX,scannerImage.curY)
        }
        onRotationChanged: {
            if(!rotani.running) {
                rotateTo = rotation
            }
        }
        onSourceSizeChanged: {  // ----------------
            curX = scannerImageItem.width/2 - width/2
            curY = scannerImageItem.height/2 - height/2
            scannerBasefunction.imageActuralLeftUpPos(scannerImage,scannerImage.curX,scannerImage.curY)
        }
        onScaleChanged: {
            scannerBasefunction.imageActuralLeftUpPos(scannerImage,scannerImage.curX,scannerImage.curY)
        }

        onStatusChanged: { //每次重新加载图片进入时重置位置
            if (scannerImage.status == Image.Ready) {
                if (newImageLoaded) {
                    scannerImage.scannerImagePostionReset()
                }
            }
        }

        //动画
        Behavior on x {
            PropertyAnimation {
                id: xani
                duration: startAni ? scannerVariables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }
        Behavior on y {
            PropertyAnimation {
                id: yani
                duration: startAni ? scannerVariables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }
        Behavior on scale {
            PropertyAnimation {
                id: scaleani
                duration: startAni ? scannerVariables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }
        Behavior on rotation {
            PropertyAnimation {
                id: rotani
                duration: startAni ? scannerVariables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }

        //设置图片缩放比
        function sendScannerImageScale() {
            return scannerBasefunction.getImageScale(scannerImage)
        }
        function changeScannerScale(scaleValue) {
            scannerBasefunction.setImageScaleWithImageUpdate(scannerImage,scaleValue)
        }
        //重设位置
         function scannerImagePostionReset() {
             scannerBasefunction.resetImagePostion(scannerImage)
         }
        //设置图片路径
        function changeScannerImage(imageUrl,imageWidth,imageHeight) {
            if (imageUrl === "") {
                return
            }
            realWidth = imageWidth
            realHeight = imageHeight
            if (scannerVariables.operateImageWay === 12) {
                console.log("图片操作方式 = 12 切图")
                newImageLoaded = true
                theImage.source = ""
                startAni = false
                theImage.x = scannerVariables.curWidW / 2 - imageWidth / 2
                theImage.y = scannerVariables.curWidH / 2 - imageHeight / 2
                theImage.source = imageUrl
                startAni = true
            }

        }

        function getOperateImageWay() {
            return scannerVariables.operateImageWay
        }
    }

    PinchArea {
        id: pincharea
        anchors.fill: scannerImage
        scale: scannerImage.scale
        rotation: scannerImage.rotation
        enabled: true
        property real initialScale  //触摸事件的实际比例因子是初始比例乘以Pinch.scale

        onPinchStarted: {
            initialScale = scannerImage.curScale
        }
        onPinchUpdated: {
            startAni = false
            if (!imageNeedCenterZoom) {
                scannerBasefunction.performZoom(scannerImage,scannerImage.mapFromItem(pincharea,pinch.center.x, pinch.center.y), undefined, false, false, true, (initialScale*pinch.scale)/scannerImage.curScale)
            } else {
                scannerBasefunction.performZoom(scannerImage,scannerImage.mapFromItem(pincharea, scannerWindow.width/2, scannerWindow.height/2), undefined, false, false, true,(initialScale*pinch.scale)/scannerImage.curScale)
                scannerImage.scannerImagePostionReset()
            }
            startAni = true
        }

        MouseArea {
            id: mousearea
            enabled: mouseareaEnabeled
            anchors.fill: parent
            hoverEnabled: false // 必须设置为false，否则，鼠标的坐标抓取的就不是全局的
            propagateComposedEvents: true //事件传递
        }
    }

    Connections {
        target: scannerContainer
        onZoomOut: {
            if(Math.abs(scannerImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 ||
               Math.abs(scannerImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270)
            {
                if ((selectionRect.width + 2 >= scannerImage.height*scannerImage.curScale) ||
                        (selectionRect.height + 2 >= scannerImage.width*scannerImage.curScale))
                {
                    scannerBasefunction.imageActuralLeftUpPos(scannerImage,scannerImage.curX,scannerImage.curY)
                    selectionRect.x = imageLeftUpX
                    selectionRect.y = imageLeftUpY
                    if (selectionRect.width > scannerImage.height*scannerImage.curScale)
                       selectionRect.width = scannerImage.height*scannerImage.curScale-1
                    if (selectionRect.height > scannerImage.width*scannerImage.curScale)
                       selectionRect.height = scannerImage.width*scannerImage.curScale-1
                    selectionRect.updateStartAndEndPoint()
                    return
                }
            }
            else
            {
                if ((selectionRect.width + 2 >= scannerImage.width*scannerImage.curScale) ||
                        (selectionRect.height + 2 >= scannerImage.height*scannerImage.curScale))
                {
                    scannerBasefunction.imageActuralLeftUpPos(scannerImage,scannerImage.curX,scannerImage.curY)
                    selectionRect.x = imageLeftUpX
                    selectionRect.y = imageLeftUpY
                    if (selectionRect.width > scannerImage.width*scannerImage.curScale)
                       selectionRect.width = scannerImage.width*scannerImage.curScale-1
                    if (selectionRect.height > scannerImage.height*scannerImage.curScale)
                       selectionRect.height = scannerImage.height*scannerImage.curScale-1
                    selectionRect.updateStartAndEndPoint()
                    return
                }
            }

            if (needWheelReact == false) {
                return
            }

            sigWheelZoom()
            if (wheelDelta != undefined && !imageNeedCenterZoom) {
                scannerBasefunction.performZoom(scannerImage,scannerImage.mapFromItem(scannerBgImage, scannerVariables.mousePos.x, scannerVariables.mousePos.y), undefined, false, false, false)
            } else {
                scannerBasefunction.performZoom(scannerImage,scannerImage.mapFromItem(scannerBgImage, scannerWindow.width/2, scannerWindow.height/2), undefined, false, false, false)
                scannerImage.scannerImagePostionReset()
            }
            scannerBasefunction.makeSureImageOverWid(scannerImage)
            scannerVariables.operateImageWay = 2
            changeOperateWay()
            scannerVariables.imageZoomSpeed = 15

            if(Math.abs(scannerImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 ||
               Math.abs(scannerImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270)
            {
                if (selectionRect.x < imageLeftUpX)
                {
                    selectionRect.x = imageLeftUpX
                }
                else if (selectionRect.x+selectionRect.width>imageLeftUpX+scannerImage.height*scannerImage.curScale)
                {
                    selectionRect.x = imageLeftUpX+scannerImage.height*scannerImage.curScale-selectionRect.width
                }

                if (selectionRect.y < imageLeftUpY)
                {
                    selectionRect.y = imageLeftUpY
                }
                else if (selectionRect.y+selectionRect.height>imageLeftUpY+scannerImage.width*scannerImage.curScale)
                {
                    selectionRect.y = imageLeftUpY+scannerImage.width*scannerImage.curScale-selectionRect.height
                }
            }
            else
            {
                if (selectionRect.x < imageLeftUpX)
                {
                    selectionRect.x = imageLeftUpX
                }
                else if (selectionRect.x+selectionRect.width>imageLeftUpX+scannerImage.width*scannerImage.curScale)
                {
                    selectionRect.x = imageLeftUpX+scannerImage.width*scannerImage.curScale-selectionRect.width
                }

                if (selectionRect.y < imageLeftUpY)
                {
                    selectionRect.y = imageLeftUpY
                }
                else if (selectionRect.y+selectionRect.height>imageLeftUpY+scannerImage.height*scannerImage.curScale)
                {
                    selectionRect.y = imageLeftUpY+scannerImage.height*scannerImage.curScale-selectionRect.height
                }
            }
        }
        onZoomIn: {
            if (needWheelReact == false) {
                return
            }
            sigWheelZoom()
            if (wheelDelta != undefined && !imageNeedCenterZoom) {
                scannerBasefunction.performZoom(scannerImage,scannerImage.mapFromItem(scannerBgImage, scannerVariables.mousePos.x, scannerVariables.mousePos.y), undefined, true, false, false)
            } else {
                scannerBasefunction.performZoom(scannerImage,scannerImage.mapFromItem(scannerBgImage, scannerWindow.width/2, scannerWindow.height/2), undefined, true, false, false)
                scannerImage.scannerImagePostionReset()
            }
            scannerBasefunction.makeSureImageOverWid(scannerImage)
            scannerVariables.operateImageWay = 1
            changeOperateWay()
            scannerVariables.imageZoomSpeed = 15
        }
        onRotate: {
            scannerImage.rotateTo += deg
        }
        onMirrorH: {
            var old = scannerImage.mirror
            scannerImage.mirror = !old
            startAni = false
            if (scannerImage.rotateTo / scannerVariables.angle90 % 2 != 0) {
                scannerImage.rotateTo += scannerVariables.angle180
            }
            startAni = true
        }
        onMirrorV: {
            var old = scannerImage.mirror
            scannerImage.mirror = !old
            startAni = false
            if (scannerImage.rotateTo / scannerVariables.angle90 % 2 == 0) {
                scannerImage.rotateTo += scannerVariables.angle180
            }
            startAni = true
        }
    }

    Connections {
        target: scannerVariables
        onOperateImageWayChanged: {

            if (scannerVariables.operateImageWay === 3) {           //逆时针旋转
                startAni = true
                scannerContainer.rotate(-scannerVariables.angle90)
                scannerImage.scannerImagePostionReset()
                startAni = false
            } else if (scannerVariables.operateImageWay === 4) {    //顺时针旋转
                startAni = true
                scannerContainer.rotate(scannerVariables.angle90)
                scannerImage.scannerImagePostionReset()
                startAni = false
            } else if (scannerVariables.operateImageWay === 5) {    //水平
                scannerContainer.mirrorH()
                scannerImage.scannerImagePostionReset()
            } else if (scannerVariables.operateImageWay === 6) {    //垂直
                scannerContainer.mirrorV()
                scannerImage.vPostionReset()
            }

            //裁剪
            if (scannerVariables.operateImageWay === 10) {
                isCutting = true;
                canOperate = true;
                selectionRect.visible = false;
                selectionRect.width = 0;
                selectionRect.height = 0;
            }
            //退出裁剪
            if ( scannerVariables.operateImageWay === 13) {
                isCutting = false;
                selectionRect.visible = false;
                selectionRect.width = 0;
                selectionRect.height = 0;

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
            drag.maximumX: imageLeftUpX+(Math.abs(scannerImage.rotateTo % scannerVariables.angle360) === scannerVariables.angle90 ||
                                         Math.abs(scannerImage.rotateTo % scannerVariables.angle360) === scannerVariables.angle270 ? scannerImage.height : scannerImage.width)*scannerImage.curScale-parent.width
            drag.minimumY: imageLeftUpY
            drag.maximumY: imageLeftUpY+(Math.abs(scannerImage.rotateTo % scannerVariables.angle360) === scannerVariables.angle90 ||
                                         Math.abs(scannerImage.rotateTo % scannerVariables.angle360) === scannerVariables.angle270 ? scannerImage.width : scannerImage.height)*scannerImage.curScale-parent.height
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
                selectionRect.x = Math.min(point.x, selectionRect.endPoint.x);
                selectionRect.y = Math.min(point.y, selectionRect.endPoint.y);
                selectionRect.width = Math.max(selectionRect.endPoint.x, point.x)  - selectionRect.x;
                selectionRect.height = Math.max(selectionRect.endPoint.y, point.y) - selectionRect.y;
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
                selectionRect.x = Math.min(point.x, selectionRect.endPoint.x);
                selectionRect.width = Math.max(selectionRect.endPoint.x, point.x)  - selectionRect.x;
                selectionRect.height = Math.max(selectionRect.startPoint.y, point.y) - selectionRect.y;
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
                selectionRect.y = Math.min(point.y, selectionRect.endPoint.y);
                selectionRect.width = Math.max(selectionRect.startPoint.x, point.x)  - selectionRect.x;
                selectionRect.height = Math.max(selectionRect.endPoint.y, point.y) - selectionRect.y;
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
                selectionRect.width = Math.max(selectionRect.startPoint.x, point.x)  - selectionRect.x;
                selectionRect.height = Math.max(selectionRect.startPoint.y, point.y) - selectionRect.y;
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
                selectionRect.y = Math.min(point.y, selectionRect.endPoint.y);
                selectionRect.height = Math.max(selectionRect.endPoint.y, point.y) - selectionRect.y;
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
                selectionRect.height = Math.max(selectionRect.startPoint.y, point.y) - selectionRect.y;
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
                selectionRect.x = Math.min(point.x, selectionRect.endPoint.x);
                selectionRect.width = Math.max(selectionRect.endPoint.x, point.x)  - selectionRect.x;
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
                selectionRect.width = Math.max(selectionRect.startPoint.x, point.x)  - selectionRect.x;
            }
        }
    }

    function startCut() {
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

    function setCutSize(width, height, x, y) {
        selectionRect.width = width;
        selectionRect.height = height;
        selectionRect.x = x;
        selectionRect.y = y;
        selectionRect.visible = true;
        selectionRect.updateStartAndEndPoint();

        isCutting = true;
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

        scannerBasefunction.imageActuralLeftUpPos(scannerImage,scannerImage.curX,scannerImage.curY)

        var beginX = getCutInfoX()
        var beginY = getCutInfoY()
        if ((beginX < 0) || (beginY < 0))
        {
            return false;
        }

        var visibleW
        var visibleH
        if(Math.abs(scannerImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 ||
           Math.abs(scannerImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270) {
            visibleH = scannerImage.width * scannerImage.curScale
            visibleW =  scannerImage.height * scannerImage.curScale
        } else {
            visibleW = scannerImage.width * scannerImage.curScale
            visibleH =  scannerImage.height * scannerImage.curScale
        }

        var endX = beginX + selectionRect.width
        var endY = beginY + selectionRect.height
        if ((endX > visibleW) || (endY > visibleH)) {
            return false;
        }

        return true;
    }


    function getRotateTo(){
        return scannerImage.rotateTo;
    }

    function scannerOperateImage(operateWay) {
        scannerVariables.operateWay = operateWay
        //放大缩小
        if (operateWay === 1) {
            scannerContainer.zoomIn(undefined)
        } else if (operateWay === 2) {
            scannerContainer.zoomOut(undefined)
        }
        //切图
        if (operateWay === 12) {
            startAni = false
            scannerImage.rotateTo = 0
            scannerImage.mirror = false
            newImageLoaded = true
            scannerImage.scannerImagePostionReset()

        } else {
            newImageLoaded = false
        }
        //原始尺寸
        if (operateWay === 8) {
            scannerImage.curScale = 1
            scannerImage.scannerImagePostionReset()
        }
    }

    function comparePressedShowImage() {
        scannerImage.curRotaion = scannerImage.rotation
        scannerImage.curMirror = scannerImage.mirror
    }
    function compareReleasedShowImage() {
        scannerImage.rotateTo = scannerImage.curRotaion
        scannerImage.mirror = scannerImage.curMirror
        scannerImage.scannerImagePostionReset()
    }
}
