// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import org.deepin.image.viewer 1.0 as IV
import "./Utils"

Item {
    id: fullThumbnail

    // 是否启用动画效果，用于强制更新组件位置而不使用动画效果
    property bool enableAnimation: false

    // 鼠标是否进入当前的视图
    property bool isEnterCurrentView: false
    // 是否标题栏和底栏需要隐藏(仅判断普通模式)
    property bool needBarHideInNormalMode: false

    //判断工具栏和标题栏的显示隐藏
    function animationAll() {
        if (IV.GStatus.animationBlock) {
            return;
        }

        // 打开界面不计算标题栏显隐
        if (IV.GStatus.stackPage === Number(IV.Types.OpenImagePage)) {
            return;
        }

        // 根据当前不同捕获行为获取光标值
        var mouseX = imageViewerArea.usingCapture ? imageViewerArea.captureX : imageViewerArea.mouseX;
        var mouseY = imageViewerArea.usingCapture ? imageViewerArea.captureY : imageViewerArea.mouseY;

        // 判断光标是否离开了窗口
        var cursorInWidnow = mouseX >= 0 && mouseX <= Window.width && mouseY >= 0 && mouseY <= Window.height;
        // 工具栏显示的热区高度，窗口高度 - 工具栏距底部高度(工具栏高 70px + 边距 10px)
        var bottomHotspotHeight = Window.height - IV.GStatus.showBottomY;
        // 按缩放比例计算是否需要显示标题/工具栏，默认显示
        var imageScaleNeedShow = true;
        if (imageViewer.targetImageReady) {
            // 显示图像的像素高度
            var imagePaintedHeight = imageViewer.targetImage.paintedHeight * imageViewer.targetImage.scale;
            // 显示图像的组件高度(组件高度不会随着缩放变更，是组件在布局内的高度)
            var imageCompoHeight = imageViewer.targetImage.height;
            imageScaleNeedShow = Boolean(imagePaintedHeight <= imageCompoHeight);

            // 设置标题栏的状态，是否图片已缩放到标题栏部分
            titleRect.imageScaledToTitle = !imageScaleNeedShow;
        }

        // 判断标题栏和底栏的计算
        if (window.isFullScreen) {
            // 全屏时特殊处理
            if (mouseY > bottomHotspotHeight) {
                thumbnailViewBackGround.animationShow = true;
            } else {
                titleRect.animationShow = false;
                thumbnailViewBackGround.animationShow = false;
            }
        } else {
            // 判断是否弹出标题栏和缩略图栏
            var needShowTopBottom = false;
            if ((Window.height <= IV.GStatus.minHideHeight || Window.width <= IV.GStatus.minWidth) && (mouseY <= bottomHotspotHeight) && (mouseY >= IV.GStatus.titleHeight)) {
                // 窗口大小大于最小大小，光标不在热区内
                needShowTopBottom = false;
            } else if (imageScaleNeedShow) {
                // 缩放范围高度未超过显示范围高度限制时，不会隐藏工具/标题栏，根据高度而非宽度计算
                needShowTopBottom = true;
            } else if (cursorInWidnow && ((mouseY > bottomHotspotHeight && mouseY <= Window.height) || (0 < mouseY && mouseY < IV.GStatus.titleHeight))) {
                // 当缩放范围超过工具/标题栏且光标在工具/标题栏范围，显示工具/标题栏
                needShowTopBottom = true;
            } else {
                needShowTopBottom = false;
            }
            titleRect.animationShow = needShowTopBottom;
            thumbnailViewBackGround.animationShow = needShowTopBottom;
            needBarHideInNormalMode = !needShowTopBottom;
        }

        // 光标不在切换按钮纵向判断的热区(处于标题栏/工具栏区域)时，隐藏左右切换按钮
        // 判断是否弹出图片切换按钮
        var needShowLeftRightBtn = false;
        if (IV.GStatus.titleHeight < mouseY && mouseY < bottomHotspotHeight && isEnterCurrentView && cursorInWidnow) {
            if (mouseX >= Window.width - IV.GStatus.switchImageHotspotWidth && mouseX <= Window.width) {
                // 光标处于切换下一张按钮区域
                needShowLeftRightBtn = true;
            } else if (mouseX <= IV.GStatus.switchImageHotspotWidth && mouseX >= 0) {
                // 光标处于切换上一张按钮区域
                needShowLeftRightBtn = true;
            }
        }
        floatLeftButton.animationShow = needShowLeftRightBtn;
        floatRightButton.animationShow = needShowLeftRightBtn;
    }

    //判断工具栏和标题栏的显示隐藏
    function changeSizeMoveAll() {
        // 打开界面不计算标题栏显隐
        if (IV.GStatus.stackPage === Number(IV.Types.OpenImagePage)) {
            return;
        }

        // 工具栏显示的热区高度，窗口高度 - 工具栏距底部高度(工具栏高 70px + 边距 10px)
        var bottomHotspotHeight = Window.height - IV.GStatus.showBottomY;

        // 按缩放比例计算是否需要显示标题/工具栏
        var imageScaleNeedShow = true;
        if (imageViewer.targetImageReady) {
            // 显示图像的像素高度
            var imagePaintedHeight = imageViewer.targetImage.paintedHeight * imageViewer.targetImage.scale;
            // 显示图像的组件高度(组件高度不会随着缩放变更，是组件在布局内的高度)
            var imageCompoHeight = imageViewer.targetImage.height;
            imageScaleNeedShow = Boolean(imagePaintedHeight <= imageCompoHeight);
        }

        // 变更大小时的位置变更不触发动画效果
        fullThumbnail.enableAnimation = false;

        // 刷新标题栏/底部栏的位置
        if (window.isFullScreen) {
            if (imageViewerArea.mouseY > bottomHotspotHeight) {
                thumbnailViewBackGround.animationShow = true;
            } else {
                titleRect.animationShow = false;
                thumbnailViewBackGround.animationShow = false;
            }
        } else if ((Window.height <= IV.GStatus.minHideHeight || Window.width <= IV.GStatus.minWidth) && (imageViewerArea.mouseY <= bottomHotspotHeight) && imageViewerArea.mouseY >= IV.GStatus.titleHeight) {
            titleRect.animationShow = false;
            thumbnailViewBackGround.animationShow = false;
        } else if (imageViewerArea.mouseY > bottomHotspotHeight || imageViewerArea.mouseY < IV.GStatus.titleHeight || imageScaleNeedShow) {
            titleRect.animationShow = true;
            thumbnailViewBackGround.animationShow = true;
        } else {
            titleRect.animationShow = false;
            thumbnailViewBackGround.animationShow = false;
        }
        thumbnailViewBackGround.updatePosition();

        // 刷新左右切换按钮的位置
        var showLeftRightButton = false;
        if (imageViewerArea.mouseX <= 100 && imageViewerArea.mouseX <= Window.width && isEnterCurrentView) {
            showLeftRightButton = true;
        } else if (imageViewerArea.mouseX >= Window.width - 100 && imageViewerArea.mouseX >= 0 && isEnterCurrentView) {
            showLeftRightButton = true;
        }
        floatLeftButton.animationShow = showLeftRightButton;
        floatRightButton.animationShow = showLeftRightButton;
        floatLeftButton.updatePosition();
        floatRightButton.updatePosition();
        fullThumbnail.enableAnimation = true;
    }

    // 切换标题栏和工具栏显示状态
    function switchTopAndBottomBarState() {
        // 判断当前标题栏、工具栏处于是否隐藏模式下
        if (needBarHideInNormalMode || window.isFullScreen) {
            //判断当前标题栏、工具栏是否已隐藏
            if (Window.height <= thumbnailViewBackGround.y) {
                // 全屏下不展示标题栏
                titleRect.animationShow = !window.isFullScreen;
                thumbnailViewBackGround.animationShow = true;
            } else {
                titleRect.animationShow = false;
                thumbnailViewBackGround.animationShow = false;
            }
        }
    }

    anchors.fill: parent

    Component.onCompleted: {
        changeSizeMoveAll();
    }
    onHeightChanged: {
        changeSizeMoveAll();
    }
    onWidthChanged: {
        changeSizeMoveAll();
    }

    ImageViewer {
        id: imageViewer

        anchors.fill: parent

        onTargetImageChanged: {
            delayAnimationTimer.start();
        }
        onTargetImageReadyChanged: {
            delayAnimationTimer.start();
        }
    }

    // 缩放变更时触发显示/隐藏标题栏/底部栏
    Connections {
        function onScaleChanged() {
            delayAnimationTimer.start();
        }

        enabled: imageViewer.targetImageReady
        target: imageViewer.targetImage
    }

    // 旋转图片时 targetImage 和 scale (1.0) 可能均不变更，获取旋转状态触发标题栏缩放
    Connections {
        function onCurrentRotationChanged() {
            delayAnimationTimer.start();
        }

        target: IV.GControl
    }

    Timer {
        id: delayAnimationTimer

        interval: 10
        repeat: false

        onTriggered: animationAll()
    }

    FloatingButton {
        id: floatLeftButton

        property bool animationShow: false

        function updatePosition() {
            floatLeftButton.x = animationShow ? 20 : -50;
        }

        checked: false
        enabled: IV.GControl.hasPreviousImage
        height: 50
        icon.name: "icon_previous"
        visible: enabled
        width: 50

        Behavior on x {
            enabled: fullThumbnail.enableAnimation

            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }

        onAnimationShowChanged: updatePosition()
        onClicked: IV.GControl.previousImage()

        anchors {
            top: parent.top
            topMargin: IV.GStatus.titleHeight + (parent.height - IV.GStatus.titleHeight - IV.GStatus.showBottomY) / 2
        }
    }

    FloatingButton {
        id: floatRightButton

        property bool animationShow: false

        function updatePosition() {
            floatRightButton.x = animationShow ? Window.width - 70 : Window.width;
        }

        checked: false
        enabled: IV.GControl.hasNextImage
        height: 50
        icon.name: "icon_next"
        visible: enabled
        width: 50

        Behavior on x {
            enabled: fullThumbnail.enableAnimation

            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }

        onAnimationShowChanged: updatePosition()
        onClicked: IV.GControl.nextImage()

        anchors {
            top: parent.top
            topMargin: IV.GStatus.titleHeight + (parent.height - IV.GStatus.titleHeight - IV.GStatus.showBottomY) / 2
        }
    }

    MouseArea {
        id: imageViewerArea

        property int captureX: 0 // 当前的光标X坐标值
        property int captureY: 0 // 当前的光标Y坐标值

        property bool usingCapture: false // 是否使用定时捕获光标位置

        acceptedButtons: Qt.LeftButton
        hoverEnabled: true

        onEntered: {
            isEnterCurrentView = true;
            animationAll();
        }
        onExited: {
            isEnterCurrentView = false;
            animationAll();

            // 当光标移出当前捕获范围时触发(不一定移出了窗口)
            IV.CursorTool.setCaptureCursor(true);
            imageViewerArea.usingCapture = true;
        }
        onMouseYChanged: mouse => {
            animationAll();
            mouse.accepted = false;
        }

        // NOTE: 不能遮挡顶部标题栏，否则无法传递鼠标消息到底部组件移动窗口
        anchors {
            fill: imageViewer
            topMargin: IV.GStatus.titleHeight
        }

        Connections {
            function onCursorPosChanged(x, y) {
                if (imageViewerArea.usingCapture) {
                    var pos = mapFromGlobal(x, y);
                    imageViewerArea.captureX = pos.x;
                    imageViewerArea.captureY = pos.y;
                    // 根据光标位置计算工具、标题、侧边栏的收缩弹出
                    animationAll();

                    // 若光标已移出界面，停止捕获光标位置
                    var cursorInWidnow = pos.x >= 0 && pos.x <= window.width && pos.y >= 0 && pos.y <= window.height;
                    if (!cursorInWidnow) {
                        IV.CursorTool.setCaptureCursor(false);
                        imageViewerArea.usingCapture = false;
                    }
                }
            }

            target: IV.CursorTool
        }
    }

    ThumbnailListView {
        id: thumbnailViewBackGround

        property bool animationShow: true

        function updatePosition() {
            thumbnailViewBackGround.y = animationShow ? Window.height - IV.GStatus.showBottomY : Window.height;
        }

        anchors.horizontalCenter: parent.horizontalCenter
        height: 70
        targetImage: imageViewer.targetImage
        // 根据拓展的列表宽度计算, 20px为工具栏和主窗口间的间距 2x10px
        width: parent.width - 20 < btnContentWidth + listContentWidth ? parent.width - 20 : btnContentWidth + listContentWidth
        y: Window.height - IV.GStatus.showBottomY

        background: FloatingPanel {
            id: control

            implicitHeight: 70
        }
        Behavior on y {
            enabled: fullThumbnail.enableAnimation

            NumberAnimation {
                duration: 366
                easing.type: Easing.OutExpo
            }
        }

        onAnimationShowChanged: {
            if (animationShow) {
                updatePosition();
            } else {
                animationQuitDelay.restart();
            }
        }

        Timer {
            id: animationQuitDelay

            interval: 500

            onTriggered: thumbnailViewBackGround.updatePosition()
        }
    }

    //浮动提示框
    FloatingNotice {
        id: floatLabel

        anchors.bottom: parent.bottom
        anchors.bottomMargin: thumbnailViewBackGround.height + IV.GStatus.floatMargin
        anchors.left: parent.left
        anchors.leftMargin: parent.width / 2 - 50
        opacity: 0.7
        visible: false

        Timer {
            interval: 1500
            repeat: false
            running: parent.visible

            onTriggered: {
                parent.visible = false;
            }
        }
    }
}
