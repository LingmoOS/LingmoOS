// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Window 2.11
import QtGraphicalEffects 1.0

import org.deepin.dtk 1.0

Item {
    id: fullThumbnail
    property alias source: imageViewer.source
    property alias sourcePaths: imageViewer.sourcePaths
    property alias currentIndex: imageViewer.swipeIndex

    signal closeFullThumbnail

    // 鼠标是否进入当前的视图
    property bool isEnterCurrentView:true
    // 是否标题栏和底栏需要隐藏(仅判断普通模式)
    property bool needBarHideInNormalMode: false

    // 从相册进入大图模式，展示上浮下边栏动画时，禁用看图原下边栏动画
    property bool noImageViewerBottomAnimation: false

    function setThumbnailCurrentIndex(index) {
        toolBarthumbnailListView.currentIndex = index
    }

    // 切换标题栏和工具栏显示状态
    function switchTopAndBottomBarState() {
        // 判断当前标题栏、工具栏处于是否隐藏模式下
        if (needBarHideInNormalMode
                || Window.FullScreen === window.visibility) {
            var curRectY = thumbnailViewBackGround.y
            //判断当前标题栏、工具栏是否已隐藏
            if (window.height <= curRectY) {
                hideTopTitleAnimation.stop()
                hideBottomAnimation.stop()

                // 全屏下不展示标题栏
                if (Window.FullScreen !== window.visibility) {
                    showTopTitleAnimation.start()
                }
                showBottomAnimation.start()
            } else {
                showTopTitleAnimation.stop()
                showBottomAnimation.stop()

                hideTopTitleAnimation.start()
                hideBottomAnimation.start()
            }
        }
    }

    //左右按钮隐藏动画
    NumberAnimation {
        id :hideLeftButtonAnimation
        target: floatLeftButton
        from: floatLeftButton.x
        to: -50
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :showLeftButtonAnimation
        target: floatLeftButton
        from: floatLeftButton.x
        to: 20
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :hideRightButtonAnimation
        target: floatRightButton
        from: floatRightButton.x
        to: parent.width
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :showRightButtonAnimation
        target: floatRightButton
        from: floatRightButton.x
        to:parent.width-70
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    //工具栏动画和标题栏动画
    NumberAnimation {
        id :hideBottomAnimation
        target: thumbnailViewBackGround
        from: thumbnailViewBackGround.y
        to: window.height
        property: "y"
        duration: 200
        easing.type: Easing.InOutQuad
    }
    NumberAnimation {
        id :hideTopTitleAnimation
        target: titleRect
        from: titleRect.y
        to: -50
        property: "y"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :showBottomAnimation
        target: thumbnailViewBackGround
        from: thumbnailViewBackGround.y
        to:  window.height-GStatus.showBottomY
        property: "y"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :showTopTitleAnimation
        target: titleRect
        from: titleRect.y
        to: 0
        property: "y"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    // 从相册双击图片进入时，展示工具栏上浮动画
    ParallelAnimation {
        id: showToolBarAnimationInAlbum
        NumberAnimation{
            target: thumbnailViewBackGround
            properties: "opacity";
            easing.type: Easing.OutExpo;
            duration: GStatus.animationDuration
            from: 0
            to: 0.5
        }

        NumberAnimation{
            target: thumbnailViewBackGround
            properties: "y";
            easing.type: Easing.OutExpo;
            duration: GStatus.animationDuration
            from: thumbnailViewBackGround.y
            to:  window.height-GStatus.showBottomY
        }

        onStopped: {
            noImageViewerBottomAnimation = false
        }
    }

    Connections {
        target: window
        onSigShowToolBar: {
            thumbnailViewBackGround.opacity = 0
            thumbnailViewBackGround.y = window.height
            noImageViewerBottomAnimation = true
            showToolBarAnimationInAlbum.start()
        }
    }

    //判断工具栏和标题栏的显示隐藏
    function animationAll() {
        // 从相册进入看图，展示下边栏动画时，不计算标题栏显隐
        if (noImageViewerBottomAnimation)
            return
        // 打开界面不计算标题栏显隐
        if (stackView.currentWidgetIndex === 0) {
            return
        }

        // 根据当前不同捕获行为获取光标值
        var mouseX = imageViewerArea.usingCapture ? imageViewerArea.captureX : imageViewerArea.mouseX;
        var mouseY = imageViewerArea.usingCapture ? imageViewerArea.captureY : imageViewerArea.mouseY;

        // 判断光标是否离开了窗口
        var cursorInWidnow = mouseX >= 0 && mouseX <= window.width && mouseY >= 0 && mouseY <= window.height
        // 显示图像的像素高度
        var viewImageHeight = window.width * (FileControl.getCurrentImageHeight() / FileControl.getCurrentImageWidth())

        if (window.visibility == Window.FullScreen){
            // 全屏时特殊处理
            if(mouseY > height-100){
                showBottomAnimation.start()
            }else{
                // 隐藏动画前结束弹出动画
                showBottomAnimation.stop()
                showTopTitleAnimation.stop()

                hideBottomAnimation.start()
                hideTopTitleAnimation.start()
            }
        } else {
            // 判断是否弹出标题栏和缩略图栏
            var needShowTopBottom = false;
            if(currentWidgetIndex != 0 &&
                 ((window.height <= GStatus.minHideHeight || window.width <= GStatus.minWidth)
                  && (mouseY <= height-100)
                  && (mouseY >= titleRect.height) )){
                needShowTopBottom = false
            }else if (imageViewer.currentScale <= (1.0 * (window.height - titleRect.height * 2) / window.height)) {
                // 缩放率小于(允许显示高度/窗口高度)的不会超过工具/标题栏
                needShowTopBottom = true
            }else if ((viewImageHeight * imageViewer.currentScale) <= (window.height - titleRect.height * 2)) {
                // 缩放范围高度未超过显示范围高度限制时时，不会隐藏工具/标题栏，根据高度而非宽度计算
                needShowTopBottom = true
            }else if(cursorInWidnow
                     && ((mouseY > height - 100 && mouseY <= height)
                         || (0 < mouseY && mouseY < titleRect.height))) {
                // 当缩放范围超过工具/标题栏且光标在工具/标题栏范围，显示工具/标题栏
                needShowTopBottom = true
            }else{
                needShowTopBottom = false
            }

            if (needShowTopBottom) {
                showBottomAnimation.start()
                showTopTitleAnimation.start()
            } else {
                // 隐藏动画前结束弹出动画
                showBottomAnimation.stop()
                showTopTitleAnimation.stop()

                hideBottomAnimation.start()
                hideTopTitleAnimation.start()
            }
            needBarHideInNormalMode = !needShowTopBottom
        }

        if(mouseX>=window.width-100 && mouseX<=window.width && isEnterCurrentView && cursorInWidnow){
            showLeftButtonAnimation.start()
            showRightButtonAnimation.start()
        }else if(mouseX<=100 && mouseX>=0 && isEnterCurrentView && cursorInWidnow){
            showLeftButtonAnimation.start()
            showRightButtonAnimation.start()
        }else {
            // 隐藏动画前结束弹出动画
            showLeftButtonAnimation.stop()
            showRightButtonAnimation.stop()

            hideLeftButtonAnimation.start()
            hideRightButtonAnimation.start()
        }
    }

    //判断工具栏和标题栏的显示隐藏
    function changeSizeMoveAll(){
        showBottomAnimation.stop()
        showTopTitleAnimation.stop()
        hideBottomAnimation.stop()
        hideTopTitleAnimation.stop()
        showRightButtonAnimation.stop()
        hideRightButtonAnimation.stop()
        if(window.visibility==Window.FullScreen ){
            if(imageViewerArea.mouseY > height-100){
                thumbnailViewBackGround.y=window.height-GStatus.showBottomY
            }else{
                thumbnailViewBackGround.y=window.height
                titleRect.y=-50
            }
        }else if(currentWidgetIndex != 0 &&
                 ((window.height<=GStatus.minHideHeight || window.width<=GStatus.minWidth)&&(imageViewerArea.mouseY <= height-100) &&imageViewerArea.mouseY >= titleRect.height )){
            thumbnailViewBackGround.y=window.height
            titleRect.y=-50
        }else if(imageViewerArea.mouseY > height-100 || imageViewerArea.mouseY<titleRect.height ||
                 (imageViewer.currentScale <= 1.0*(window.height-titleRect.height*2)/window.height)){
            thumbnailViewBackGround.y=window.height-GStatus.showBottomY
            titleRect.y=0
        }else{
            thumbnailViewBackGround.y=window.height
            titleRect.y=-50
        }

        if(imageViewerArea.mouseX<=100 && imageViewerArea.mouseX<=window.width && isEnterCurrentView){
            floatLeftButton.x=20
            floatRightButton.x=parent.width-70
        }else if(imageViewerArea.mouseX>=window.width-100 && imageViewerArea.mouseX>=0 && isEnterCurrentView){
            floatLeftButton.x=20
            floatRightButton.x=parent.width-70
        }else{
            floatLeftButton.x=-50
            floatRightButton.x=parent.width
        }
    }

    function slotShowFullScreen(){
        thumbnailViewBackGround.y=Screen.height
        floatRightButton.x=Screen.width

    }
    function slotMaxWindow(){
        thumbnailViewBackGround.y=Screen.height
        floatRightButton.x=Screen.width
    }

    onHeightChanged: {
        changeSizeMoveAll()
    }

    onWidthChanged: {
        changeSizeMoveAll()
    }

    ImageViewer {
        id: imageViewer
        anchors.fill: parent
    }
    Connections {
        target: imageViewer
        onSigWheelChange :{
            animationAll()
        }
    }


    FloatingButton {
        id:floatLeftButton
        visible: mainView.sourcePaths.length>1 && enabled
        enabled: currentIndex > 0
                || imageViewer.frameIndex > 0
        checked: false
        anchors.top: parent.top
        anchors.topMargin: GStatus.titleHeight+(parent.height-GStatus.titleHeight-GStatus.showBottomY)/2
        icon.name : "icon_previous"
        width: 50
        height: 50
        onClicked: {
            toolBarthumbnailListView.previous();
        }
        Component.onCompleted: {
            floatLeftButton.x=-50
        }
    }
    FloatingButton {
        id:floatRightButton
        checked: false
        visible: mainView.sourcePaths.length > 1 && enabled
        enabled: currentIndex < mainView.sourcePaths.length - 1
                || imageViewer.frameIndex < imageViewer.frameCount - 1
        anchors.top: parent.top
        anchors.topMargin: GStatus.titleHeight+(parent.height-GStatus.titleHeight-GStatus.showBottomY)/2
        width: 50
        height: 50
        icon.name:"icon_next"
        onClicked: {
            toolBarthumbnailListView.next();
        }
        Component.onCompleted: {
            floatRightButton.x=parent.width
        }
    }


    MouseArea {
        anchors.fill: imageViewer
        id:imageViewerArea
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true

        property bool usingCapture: false       // 是否使用定时捕获光标位置
        property int captureX: 0                // 当前的光标X坐标值
        property int captureY: 0                // 当前的光标Y坐标值

        onMouseYChanged: {
            animationAll()
            mouse.accepted = false;
        }

        onEntered: {
            isEnterCurrentView = true
            animationAll()
        }

        onExited: {
            isEnterCurrentView = false
            animationAll()

            // 当光标移出当前捕获范围时触发(不一定移出了窗口)
            cursorTool.setCaptureCursor(true)
            imageViewerArea.usingCapture = true
        }

        Connections {
            target: cursorTool
            onCursorPos: {
                if (imageViewerArea.usingCapture) {
                    var pos = mapFromGlobal(x, y)
                    imageViewerArea.captureX = pos.x
                    imageViewerArea.captureY = pos.y
                    // 根据光标位置计算工具、标题、侧边栏的收缩弹出
                    animationAll()

                    // 若光标已移出界面，停止捕获光标位置
                    var cursorInWidnow = pos.x >= 0 && pos.x <= window.width && pos.y >= 0 && pos.y <= window.height
                    if (!cursorInWidnow) {
                        cursorTool.setCaptureCursor(false)
                        imageViewerArea.usingCapture = false
                    }
                }
            }
        }
    }

    Rectangle {
        id: thumbnailViewBackGround
        // 根据拓展的列表宽度计算
        width: parent.width - 30 < 460 + albumBtnWidth + toolBarthumbnailListView.listContentWidth
               ? parent.width - 30
               : 460 + albumBtnWidth + toolBarthumbnailListView.listContentWidth
        height: 70

        // 返回相册按钮宽度
        property var albumBtnWidth: FileControl.isAlbum() ? 40 * 2 : 0
        // 根据当前窗口大小可用的列表内容宽度(最小窗口宽度为 628)
        property int avaliableListViewWidth: parent.width - 30 - 460 - albumBtnWidth

        anchors.right: parent.right
        anchors.rightMargin: (parent.width-width)/2

        radius: panel.radius
        opacity: 0.5

        // 调整 FloatingPanel 内部的 BoxShadow 计算区域
        FloatingPanel {
            id: panel
            width: parent.width
            height: parent.height
            anchors.fill: parent

            Component.onCompleted: {
                animationAll()
            }
        }
    }

    ToolBarThumbnailListView {
        id: toolBarthumbnailListView
        anchors.fill: thumbnailViewBackGround
    }

    //浮动提示框
    FloatingNotice {
        id: floatLabel
        visible: false
        anchors.bottom: thumbnailViewBackGround.top
        anchors.bottomMargin: GStatus.floatMargin
        anchors.left: parent.left
        anchors.leftMargin: parent.width / 2 - 50
        opacity: 0.7

        Timer {
            interval: 1500
            running: parent.visible
            repeat: false
            onTriggered: {
                parent.visible = false
            }
        }
    }

}
