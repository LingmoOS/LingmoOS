// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import org.deepin.image.viewer 1.0 as IV

Item {
    id: navigation

    property bool enableRefresh: true
    property bool imageNeedNavi: false
    property real imgBottom: 0
    // 使用浮点，避免精度丢失导致拖拽边界有细微偏差
    property real imgLeft: 0
    property real imgRight: 0
    property real imgTop: 0
    // 用于动画控制，预期的隐藏动作(用于触发缩放动效时同步导航窗口的处理)
    property real prefferHide: 0
    // 期望是否显示，同时控制动画效果
    property bool prefferVisible: IV.GStatus.enableNavigation && imageNeedNavi
    // 指向的图片对象
    property Image targetImage

    // 请求释放信号，长时间不使用的导航窗口将请求销毁
    signal requestRelease

    function refreshNaviMask() {
        if (enableRefresh) {
            delayRefreshTimer.start();
        }
    }

    function refreshNaviMaskImpl() {
        if (!targetImage) {
            imageNeedNavi = false;
            return;
        }
        // 预期的缩放比例小于1时不进行显示
        if (prefferHide) {
            imageNeedNavi = false;
            return;
        }

        // 窗口小于最小尺寸时导航功能不可用
        if (!(window.height > IV.GStatus.minHideHeight && window.width > IV.GStatus.minWidth)) {
            imageNeedNavi = false;
            return;
        }

        // 图片实际绘制大小
        var paintedWidth = targetImage.paintedWidth * targetImage.scale;
        var paintedHeight = targetImage.paintedHeight * targetImage.scale;
        // 绘制区域未超过窗口显示区域
        if (paintedWidth <= Window.width && paintedHeight <= Window.height) {
            imageNeedNavi = false;
            return;
        }
        imageNeedNavi = true;

        // 获取横坐标偏移及宽度
        var xOffset = (currentImage.width - currentImage.paintedWidth) / 2;
        if (paintedWidth < Window.width) {
            maskArea.x = xOffset;
            maskArea.width = currentImage.paintedWidth;
        } else {
            // 图片显示宽度 + 超过窗口的图片宽度偏移量
            var expandWidth = paintedWidth - Window.width;
            var xRatio = ((expandWidth / 2) - targetImage.x) / paintedWidth;
            maskArea.x = xOffset + currentImage.paintedWidth * xRatio;
            var widthRatio = Window.width / paintedWidth;
            maskArea.width = currentImage.paintedWidth * widthRatio;
        }
        var yOffset = (currentImage.height - currentImage.paintedHeight) / 2;
        if (paintedHeight < Window.height) {
            maskArea.y = yOffset;
            maskArea.height = currentImage.paintedHeight;
        } else {
            var expandHeight = paintedHeight - Window.height;
            var yRatio = ((expandHeight / 2) - targetImage.y) / paintedHeight;
            maskArea.y = yOffset + currentImage.paintedHeight * yRatio;
            var heightRatio = Window.height / paintedHeight;
            maskArea.height = currentImage.paintedHeight * heightRatio;
        }
    }

    function updateImagePositionBasedOnMask() {
        enableRefresh = false;

        // 根据按键位置更新图片展示区域
        var xOffset = maskArea.x - imgLeft;
        var yOffset = maskArea.y - imgTop;
        // 按当前蒙皮位置映射图片位置
        var xRatio = xOffset / currentImage.paintedWidth;
        var yRatio = yOffset / currentImage.paintedHeight;

        // 图片实际绘制大小
        var paintedWidth = targetImage.paintedWidth * targetImage.scale;
        var paintedHeight = targetImage.paintedHeight * targetImage.scale;
        if (paintedWidth < Window.width) {
            targetImage.x = 0;
        } else {
            // 取得比例相对偏移位置 - 超过窗口的图片显示宽度
            var imageXOffset = (paintedWidth - Window.width) / 2;
            targetImage.x = imageXOffset - paintedWidth * xRatio;
        }
        if (paintedHeight < Window.height) {
            targetImage.y = 0;
        } else {
            var imageYOffset = (paintedHeight - Window.height) / 2;
            targetImage.y = imageYOffset - paintedHeight * yRatio;
        }
        enableRefresh = true;
    }

    // 默认属性为 hide 状态，切换显示状态时将自动动画，Y轴坐标由外部设置

    height: 112
    opacity: 0.3
    scale: 0.3
    visible: false
    width: 150

    states: [
        State {
            name: "show"
            when: prefferVisible

            PropertyChanges {
                opacity: 1
                scale: 1
                target: navigation
                x: 0
                y: 0
            }
        }
    ]
    transitions: Transition {
        id: animtionTrans

        reversible: true
        to: "show"

        onRunningChanged: {
            if (running) {
                visible = true;
            } else {
                // 动画结束再隐藏
                visible = prefferVisible;
            }

            // 隐藏的导航窗口在一段时间后释放
            if (!visible) {
                delayReleaseTimer.restart();
            } else {
                delayReleaseTimer.stop();
            }
        }

        NumberAnimation {
            duration: 366
            easing.type: Easing.OutExpo
            properties: "x,y,scale,opacity"
        }
    }

    onTargetImageChanged: {
        if (targetImage) {
            // 立即刷新
            refreshNaviMaskImpl();

            // transformOrigin 需在图片中心
            if (Item.Center !== targetImage.transformOrigin) {
                console.warn("Image transform origin error, not center!");
            }
        }
    }

    Timer {
        id: delayReleaseTimer

        interval: 5000
        repeat: false

        onTriggered: navigation.requestRelease()
    }

    Timer {
        id: delayRefreshTimer

        interval: 1
        repeat: false

        onTriggered: refreshNaviMaskImpl()
    }

    Connections {
        function onPaintedHeightChanged() {
            refreshNaviMask();
        }

        function onPaintedWidthChanged() {
            refreshNaviMask();
        }

        function onScaleChanged() {
            refreshNaviMask();
        }

        function onXChanged() {
            refreshNaviMask();
        }

        function onYChanged() {
            refreshNaviMask();
        }

        enabled: undefined !== targetImage && enableRefresh
        ignoreUnknownSignals: true
        target: undefined === targetImage ? null : targetImage
    }

    // 背景图片绘制区域
    Rectangle {
        id: imageRect

        anchors.fill: parent
        color: Qt.rgba(255, 255, 255, 0.4)
        layer.enabled: true
        radius: 10

        layer.effect: OpacityMask {
            maskSource: Rectangle {
                height: imageRect.height
                radius: imageRect.radius
                width: imageRect.width
            }
        }

        Image {
            id: currentImage

            function updateMask() {
                if (Image.Ready === currentImage.status) {
                    imgLeft = (currentImage.width - currentImage.paintedWidth) / 2;
                    imgTop = (currentImage.height - currentImage.paintedHeight) / 2;
                    imgRight = imgLeft + currentImage.paintedWidth;
                    imgBottom = imgTop + currentImage.paintedHeight;
                    refreshNaviMaskImpl();
                }
            }

            function updateSource() {
                source = "image://ImageLoad/" + IV.GControl.currentSource + "#frame_" + IV.GControl.currentFrameIndex;
            }

            anchors.fill: parent
            asynchronous: true
            cache: false
            fillMode: Image.PreserveAspectFit
            source: "image://ImageLoad/" + IV.GControl.currentSource + "#frame_" + IV.GControl.currentFrameIndex

            // QML6 Image Ready 时 paintedGeometry 不一定更新，调整 onStatusChanged 为 onPaintedGeometryChanged
            onPaintedGeometryChanged: updateMask()
            onSourceChanged: updateMask()
        }

        // 旋转图片触发更新导航窗口，重设图片后绑定会失效，因此手动触发图片源更新
        Connections {
            function onCurrentFrameIndexChanged() {
                currentImage.updateSource();
            }

            function onCurrentRotationChanged() {
                var temp = currentImage.source;
                currentImage.source = "";
                currentImage.source = temp;
                currentImage.updateMask();
            }

            function onCurrentSourceChanged() {
                currentImage.updateSource();
            }

            target: IV.GControl
        }
    }

    // 退出按钮
    ToolButton {
        height: 22
        width: 22
        z: 100

        background: Rectangle {
            radius: 50
        }

        onClicked: {
            IV.GStatus.enableNavigation = false;
        }

        anchors {
            right: parent.right
            rightMargin: 3
            top: parent.top
            topMargin: 3
        }

        Image {
            anchors.fill: parent
            source: "qrc:/res/close_hover.svg"
        }
    }

    // 显示范围蒙皮
    Rectangle {
        id: maskArea

        border.color: "white"
        border.width: 1
        color: "black"
        opacity: 0.4
    }

    // 允许拖动范围
    MouseArea {
        id: mouseArea

        anchors.fill: parent
        drag.axis: Drag.XAndYAxis
        drag.maximumX: imgRight - maskArea.width
        drag.maximumY: imgBottom - maskArea.height
        // 以图片的范围来限制拖动范围
        drag.minimumX: imgLeft
        drag.minimumY: imgTop
        drag.target: maskArea

        onPositionChanged: {
            if (mouseArea.pressed) {
                updateImagePositionBasedOnMask();
            }
        }

        // 拖拽与主界面的联动
        onPressed: {
            maskArea.x = Math.max(mouseX - maskArea.width / 2, 0);
            maskArea.y = Math.max(mouseY - maskArea.height / 2, 0);
            // 限定鼠标点击的蒙皮在图片内移动
            maskArea.x = Math.max(imgLeft, Math.min(maskArea.x, imgRight - maskArea.width));
            maskArea.y = Math.max(imgTop, Math.min(maskArea.y, imgBottom - maskArea.height));

            // 根据按键位置更新图片展示区域
            updateImagePositionBasedOnMask();
        }
    }
}
