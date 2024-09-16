// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV
import "../Utils"

BaseImageDelegate {
    id: delegate

    property bool rotationRunning: false

    function resetSource() {
        // 加载完成，触发动画效果
        var temp = image.source;
        image.source = "";
        // 重置初始状态
        delegate.inited = false;
        image.source = temp;
    }

    function updateSource() {
        if (delegate.source != "") {
            // 由于会 resetSource() 破坏绑定，因此重新设置源数据
            image.source = "image://ImageLoad/" + delegate.source + "#frame_" + delegate.frameIndex;
        } else {
            image.source = "";
        }
    }

    inputHandler: imageInput
    status: image.status
    targetImage: image

    onFrameIndexChanged: updateSource()
    onSourceChanged: updateSource()

    Image {
        id: image

        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectFit
        height: delegate.height
        mipmap: true
        scale: 1.0
        smooth: true
        source: "image://ImageLoad/" + delegate.source + "#frame_" + delegate.frameIndex
        width: delegate.width

        onStatusChanged: {
            if (Image.Ready === image.status && !rotationRunning) {
                rotateAnimationLoader.active = false;
            }
        }
    }

    // 旋转动画效果
    Loader {
        id: rotateAnimationLoader

        active: false
        anchors.fill: parent

        sourceComponent: Item {
            id: rotateItem

            property real previousRealWidth: 0

            function calcAnimation() {
                rotationAnimation.to = IV.GControl.currentRotation;
                // 初始化缩放比后再允许动画
                imageProxy.scale = image.scale;
                imageScaleBehavior.enabled = true;

                // 记录之前的绘制宽度，用于计算缩放比例
                previousRealWidth = image.paintedWidth;

                // 触发动画
                aniamtion.start();
            }

            anchors.fill: parent

            Connections {
                function onPaintedWidthChanged() {
                    // 注意宽高交换，缩放比 = 实际显示的高度(绘制高度 * 缩放比) / 之前绘制的高度
                    // 因此缩放的图片也能正常旋转匹配
                    imageProxy.scale = (image.paintedHeight * image.scale) / rotateItem.previousRealWidth;
                }

                target: image
            }

            ShaderEffectSource {
                id: imageProxy

                anchors.centerIn: parent
                height: image.height
                live: false
                sourceItem: image
                width: image.width

                Behavior on scale {
                    id: imageScaleBehavior

                    enabled: false

                    NumberAnimation {
                        id: scaleAnimation

                        duration: IV.GStatus.animationDefaultDuration - delayUpdate.interval
                        easing.type: Easing.OutExpo
                    }
                }

                Component.onCompleted: {
                    scheduleUpdate();
                    // 计算动画参数并触发动画
                    calcAnimation();
                }
            }

            Timer {
                id: delayUpdate

                interval: 50

                onTriggered: {
                    delegate.resetSource();
                }
            }

            // 并行动画
            ParallelAnimation {
                id: aniamtion

                alwaysRunToEnd: true

                onRunningChanged: {
                    if (running) {
                        image.visible = false;
                        delayUpdate.start();
                    }
                    if (!running && Image.Ready === image.status) {
                        image.visible = true;
                        rotateAnimationLoader.active = false;
                    }
                    rotationRunning = running;
                }

                RotationAnimation {
                    id: rotationAnimation

                    direction: RotationAnimation.Shortest
                    duration: IV.GStatus.animationDefaultDuration
                    easing.type: Easing.OutExpo
                    target: imageProxy
                }

                NumberAnimation {
                    duration: IV.GStatus.animationDefaultDuration
                    easing.type: Easing.OutExpo
                    properties: "x, y"
                    target: imageProxy
                    to: 0
                }
            }
        }
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        isRotatable: IV.FileControl.isRotatable(delegate.source)
        targetImage: image.status === Image.Ready ? image : null
    }

    Connections {
        function onChangeRotationCacheBegin() {
            // Note: 确保缓存中的数据已刷新后更新界面
            // 0 为复位，缓存中的数据已转换，无需再次加载
            if (0 !== IV.GControl.currentRotation) {
                // 激活旋转动画加载器
                rotateAnimationLoader.active = true;
            }
        }

        enabled: isCurrentImage
        target: IV.GControl
    }
}
