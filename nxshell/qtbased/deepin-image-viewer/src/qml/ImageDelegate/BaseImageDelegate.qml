// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml
import QtQuick.Effects
import org.deepin.image.viewer 1.0 as IV
import "../Utils"

Item {
    id: baseDelegate

    // 动画时调整显示距离的偏移量
    property real animationOffsetWidth: {
        // 默认的 PathView 显示在路径中的 item 有 3 个， 两侧显示在 PathView 1/6 或 5/6 的位置，图片居中显示
        if (paintedPaddingWidth > 0) {
            // 此处计算考虑 PathView 的 Path 和 Item 显示宽度等
            return -(width / IV.GStatus.pathViewItemCount) + paintedPaddingWidth;
        }
        return 0;
    }
    property int frameIndex: 0

    // 图片绘制后是否初始化，Qt6后 Image.Ready 时 PaintedWidth 没有更新，以此标志判断是否执行初始化
    property bool inited: false
    property ImageInputHandler inputHandler: null
    // PathView下不再提供 index === IV.GControl.currentIndex
    property bool isCurrentImage: parent.PathView.view.currentIndex === model.index
    // 坐标偏移，用于动画效果时调整显示位置
    property real offset: 0
    // 图片绘制区域到边框的位置
    property real paintedPaddingWidth: 0
    property url source
    property int status: Image.Null
    property Image targetImage
    property alias targetImageInfo: imageInfo
    property int type: IV.Types.NullImage

    function reset() {
        if (targetImage.paintedWidth > 0) {
            resetCache();
        }
        if (targetImage) {
            targetImage.rotation = 0;
        }

        // 复位图片拖拽状态，多页图将在子 Delegate 单独处理
        if (inputHandler) {
            inputHandler.reset();
        }
    }

    function resetCache() {
        if (inited || null === targetImage || undefined === targetImage) {
            return;
        }
        if (targetImage.paintedWidth <= 0) {
            return;
        }

        // 重置缩放
        if (targetImageInfo.scale <= 0) {
            // 匹配缩放处理，对于动图，首次加载传入的 paintedWidth 可能为 0
            if (imageInfo.width < targetImage.width && imageInfo.height < targetImage.height) {
                targetImage.scale = imageInfo.width / targetImage.paintedWidth;
            } else {
                targetImage.scale = 1;
            }
            targetImageInfo.scale = targetImage.scale;
        } else {
            targetImage.scale = targetImageInfo.scale;
        }
        targetImage.x = targetImageInfo.x;
        targetImage.y = targetImageInfo.y;
        inited = true;
    }

    function updateOffset() {
        // 需要考虑缩放时的处理（cache中缓存缩放值时优先采用cache）
        var realWidth = targetImage.paintedWidth * (targetImageInfo.scale <= 0 ? targetImageInfo.scale : targetImage.scale);
        // 图片加载过程时，图片可能未加载完成，调整默认的缩放比值以获取近似值
        if (realWidth <= 0) {
            if (imageInfo.width < baseDelegate.width && imageInfo.height < baseDelegate.height) {
                realWidth = imageInfo.width;
            } else {
                // 存在缩放可能，计算可能的宽度
                var scaleWidthRatio = baseDelegate.width / imageInfo.width;
                var scaleHeightRatio = baseDelegate.height / imageInfo.height;
                var scaleRatio = Math.min(scaleWidthRatio, scaleHeightRatio);
                realWidth = scaleRatio * imageInfo.width;
            }
        }

        // 更新绘制边距，用于动画时对齐边界
        paintedPaddingWidth = (width - realWidth) / 2;
    }

    // 动画时调整显示距离 ( 前一张图片 (-1) <-- 当前图片 (0) --> 下一张图片 (1) )
    x: animationOffsetWidth * offset

    // TODO: 优化绑定计算
    // 根据当前图片在 PathView 中的相对距离计算是否还原显示参数
    onOffsetChanged: {
        if (offset < -0.99 || 0.99 < offset) {
            reset();
        }
    }
    onStatusChanged: {
        if (Image.Ready === status || Image.Error === status) {
            // 重置状态
            reset();
        }
    }

    Connections {
        // 用于绘制更新后缩放等处理
        function onPaintedWidthChanged() {
            if (!inited) {
                reset();
            }
            updateOffset();
        }

        function onScaleChanged() {
            updateOffset();

            // 更新缓存状态
            targetImageInfo.scale = targetImage.scale;
        }

        function onSourceChanged() {
            inited = false;
        }

        function onStatusChanged() {
            if (Image.Ready === targetImage.status) {
                resetCache();
            }
        }

        function onXChanged() {
            targetImageInfo.x = targetImage.x;
        }

        function onYChanged() {
            targetImageInfo.y = targetImage.y;
        }

        enabled: undefined !== targetImage
        target: undefined === targetImage ? null : targetImage
    }

    // 用于加载大图时的延迟显示效果
    Loader {
        id: previousLoader

        property bool needBlur: false

        active: needBlur && isCurrentImage && (Image.Loading === baseDelegate.status)
        height: parent.height
        width: parent.width
        z: parent.z + 1

        sourceComponent: Item {
            anchors.fill: parent

            Image {
                id: loadImage

                // cache会缓存数据(即便Loader重新加载)，取消此设置以正确在快速旋转/切换时从正确缓存管理中读取
                cache: false
                fillMode: Image.PreserveAspectFit
                height: parent.height
                source: "image://ThumbnailLoad/" + delegate.source + "#frame_" + delegate.frameIndex
                width: parent.width
            }

            MultiEffect {
                anchors.fill: loadImage
                blur: 1.0
                blurEnabled: true
                blurMax: 4
                scale: loadImage.scale
                source: loadImage
            }
        }

        onActiveChanged: {
            if (active) {
                // 应用缓存的图像设置
                scale = targetImageInfo.scale <= 0 ? 1 : targetImageInfo.scale;
                x = targetImageInfo.x;
                y = targetImageInfo.y;
            }
        }

        // 短时间完成加载的图片内无需模糊延迟效果
        Timer {
            interval: 20
            running: Image.Loading === baseDelegate.status

            onTriggered: {
                previousLoader.needBlur = true;
            }
        }
    }

    IV.ImageInfo {
        id: imageInfo

        frameIndex: baseDelegate.frameIndex
        source: baseDelegate.source

        onInfoChanged: {
            inited = false;
        }
    }
}
