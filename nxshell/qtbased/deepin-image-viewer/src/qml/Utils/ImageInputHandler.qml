// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import org.deepin.image.viewer 1.0 as IV

// TODO: 旧版由于 tiff 图片特殊处理，部分 Delegate 单独设置了 ImageInputHandler
//      后续应移动至外部，无需重复创建
Item {
    id: imageInput

    // 仅部分图片允许旋转
    property bool isRotatable: false
    property Image targetImage: null

    function reset() {
        // 复位时立即刷新
        mouseArea.updateDragRect();
    }

    // 绘制区域变更时，刷新区域
    Connections {
        function onPaintedHeightChanged() {
            mouseArea.delayUpdateDragRect();
        }

        function onPaintedWidthChanged() {
            mouseArea.delayUpdateDragRect();
        }

        function onScaleChanged() {
            mouseArea.delayUpdateDragRect();
        }

        enabled: null !== targetImage
        target: targetImage
    }

    // 外部创建会覆盖 Flickable 组件
    MouseArea {
        id: mouseArea

        function delayUpdateDragRect() {
            if (null !== imageInput.targetImage) {
                if (imageInput.targetImage.scale < 1.0) {
                    updateDragRect();
                    return;
                }
            }
            delayUpdateTimer.start();
        }

        function updateDragRect() {
            if (null === imageInput.targetImage) {
                return;
            }
            if (targetImage.scale <= 1.0) {
                drag.minimumX = 0;
                drag.minimumY = 0;
                drag.maximumX = 0;
                drag.maximumY = 0;
                imageInput.targetImage.x = 0;
                imageInput.targetImage.y = 0;
            } else {
                var pixelWidthDelta = imageInput.targetImage.paintedWidth * imageInput.targetImage.scale - Window.width;
                drag.maximumX = pixelWidthDelta > 0 ? pixelWidthDelta / 2 : 0;
                drag.minimumX = -drag.maximumX;
                var pixelHeightDelta = imageInput.targetImage.paintedHeight * imageInput.targetImage.scale - Window.height;
                drag.maximumY = pixelHeightDelta > 0 ? pixelHeightDelta / 2 : 0;
                drag.minimumY = -drag.maximumY;
                imageInput.targetImage.x = Math.max(drag.minimumX, Math.min(imageInput.targetImage.x, drag.maximumX));
                imageInput.targetImage.y = Math.max(drag.minimumY, Math.min(imageInput.targetImage.y, drag.maximumY));
            }
        }

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent
        drag.axis: Drag.XAndYAxis
        drag.target: targetImage ? targetImage : undefined
        propagateComposedEvents: true

        onDoubleClicked: {
            if (IV.GStatus.stackPage === IV.Types.ImageViewPage) {
                console.warn("show full screen");
                IV.GStatus.showFullScreen = !IV.GStatus.showFullScreen;
            }
        }
        onPressAndHold: {
        }
        onPressed: mouse => {
            if (Qt.RightButton === mouse.button) {
                IV.GStatus.showRightMenu = true;
            }
        }
        onWheel: wheel => {
            if (null === imageInput.targetImage) {
                return;
            }
            var detla = wheel.angleDelta.y / 120;
            // 通过Keys缓存的状态可能不准确，在焦点移出时release事件没有正确捕获，
            // 修改为通过当前事件传入的按键按下信息判断
            if (Qt.ControlModifier & wheel.modifiers) {
                detla > 0 ? IV.GControl.previousImage() : IV.GControl.nextImage();
            } else {
                if (undefined === imageInput.targetImage) {
                    return;
                }

                // 缓存当前的坐标信息
                var mapPoint = mapToItem(imageInput.targetImage, wheel.x, wheel.y);
                if (detla > 0) {
                    imageInput.targetImage.scale = imageInput.targetImage.scale / 0.9;
                } else {
                    imageInput.targetImage.scale = imageInput.targetImage.scale * 0.9;
                }
                var restorePoint = mapFromItem(imageInput.targetImage, mapPoint.x, mapPoint.y);
                imageInput.targetImage.x = imageInput.targetImage.x - restorePoint.x + wheel.x;
                imageInput.targetImage.y = imageInput.targetImage.y - restorePoint.y + wheel.y;
                delayUpdateDragRect();
            }
        }

        Timer {
            id: delayUpdateTimer

            interval: 10
            repeat: false
            running: false

            onTriggered: mouseArea.updateDragRect()
        }
    }

    // 和MouseArea存在先后顺序，使用触摸时优先处理触摸事件
    PinchArea {
        id: imagePinchArea

        property double oldRotate: 0

        // 记录旧的缩放大小，防止拖拽时未保留当前状态
        property double oldScale: 0

        anchors.fill: parent
        enabled: null !== targetImage

        onPinchFinished: {
            // 更新界面缩放大小
            targetImage.scale = pinch.scale * imagePinchArea.oldScale;

            // 判断当前图片是否允许旋转
            if (isRotatable) {
                // 计算旋转角度，限制在旋转梯度为90度，以45度为分界点
                if (Math.abs(pinch.rotation) > 45) {
                    // 区分正反旋转方向
                    var isClockWise = pinch.rotation > 0;
                    // 计算绝对角度值
                    var rotateAngle = Math.floor((Math.abs(pinch.rotation) + 45) / 90) * 90;

                    // 触摸旋转保存属于低频率操作，可立即保存文件
                    IV.GControl.currentRotation += isClockWise ? rotateAngle : -rotateAngle;
                    IV.GControl.submitImageChangeImmediately();
                } else {
                    targetImage.rotation = imagePinchArea.oldRotate;
                }
            }
            mouseArea.updateDragRect();
        }
        onPinchStarted: {
            // 缩放和旋转都至少需要2指操作
            if (pinch.pointCount !== 2) {
                pinch.accepted = false;
                return;
            }
            oldScale = targetImage.scale;
            oldRotate = targetImage.rotation;
            pinch.accepted = true;
        }
        onPinchUpdated: {
            // 不设置边界，通过 onCurrentScaleChanged 处理限制缩放范围在 2% ~ 2000%
            targetImage.scale = pinch.scale * imagePinchArea.oldScale;
            if (isRotatable) {
                targetImage.rotation = pinch.rotation + oldRotate;
            }
        }

        // 多点触控区域，处理触摸屏上的点击、双击、长按事件
        MultiPointTouchArea {
            id: multiPointTouchArea

            // 当前实时触摸点数
            property int currentTouchPointsCount: 0
            // 判断是否允许切换标题栏和工具栏状态
            property bool enableSwitchState: true

            // 双击动作处理
            function doubleClickProcess() {
                IV.GStatus.showFullScreen = !IV.GStatus.showFullScreen;
            }

            anchors.fill: parent
            maximumTouchPoints: 3
            minimumTouchPoints: 1
            // 仅处理触摸事件，鼠标点击事件由MouseArea处理
            mouseEnabled: false

            onReleased: {
                // 触摸状态下，单指点击需要弹出隐藏的标题栏和工具栏(即立即显示)
                if (touchPoints.length === 1 && !menuHideTimer.running) {
                    if (enableSwitchState) {
                        // 还需要考虑全屏下处理
                        fullThumbnail.switchTopAndBottomBarState();
                    }
                    // 复位状态
                    enableSwitchState = true;

                    // 进行双击动作判断
                    if (doubleClicekdTimer.running) {
                        doubleClickProcess();
                        doubleClicekdTimer.stop();
                    } else {
                        doubleClicekdTimer.restart();
                    }
                }
            }

            // 获取当前实时触摸点数
            onTouchUpdated: {
                currentTouchPointsCount = touchPoints.length;
            }

            // 用于记录菜单隐藏的定时器
            Timer {
                id: menuHideTimer

                interval: 400
                running: !IV.GStatus.showRightMenu
            }

            // 双击触发定时器 双击间隔400ms内触发全屏展示
            Timer {
                id: doubleClicekdTimer

                interval: 400
            }

            // 长按触发定时器
            Timer {
                id: pressHoldTimer

                interval: 400

                // 仅一个触点按下时，延时400ms触发右键菜单
                running: multiPointTouchArea.currentTouchPointsCount === 1

                onTriggered: {
                    IV.GStatus.showImageInfo = false;
                    multiPointTouchArea.enableSwitchState = false;
                    // 弹出右键菜单
                    IV.GStatus.showRightMenu = true;
                }
            }
        }
    }
}
