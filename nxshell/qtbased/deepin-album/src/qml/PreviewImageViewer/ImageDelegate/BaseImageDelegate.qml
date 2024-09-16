// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml
import QtQuick.Effects
import org.deepin.image.viewer 1.0 as IV
import org.deepin.album 1.0 as Album
import "../Utils"

Item {
    id: baseDelegate

    // 动画时调整显示距离的偏移量
    property real animationOffsetWidth: {
        // 默认的 PathView 显示在路径中的 item 有 3 个， 两侧显示在 PathView 1/6 或 5/6 的位置，图片居中显示
        if (paintedPaddingWidth > 0) {
            // 此处计算考虑 PathView 的 Path 和 Item 显示宽度等
            return -(width / GStatus.pathViewItemCount) + paintedPaddingWidth;
        }
        return 0;
    }
    property int frameIndex: 0

    // 图片绘制后是否初始化，Qt6后 Image.Ready 时 PaintedWidth 没有更新，以此标志判断是否执行初始化
    property bool inited: false
    property ImageInputHandler inputHandler: null
    // PathView下不再提供 index === GControl.currentIndex
    property bool isCurrentImage: parent.PathView.isCurrentItem
    // 坐标偏移，用于动画效果时调整显示位置
    property real offset: 0
    // 图片绘制区域到边框的位置
    property real paintedPaddingWidth: 0
    property url source
    property int status: Image.Null
    property Image targetImage
    property alias targetImageInfo: imageInfo
    property int type: Album.Types.NullImage

    //相册进入看图动效，会改变imageviewer大小，会导致代理内的图片内容跟随缩放，这里需要消除这类缩放
    onWidthChanged: {
        if (!GStatus.enteringImageViewer)
            return
        // 匹配缩放处理，对于动图，首次加载传入的 paintedWidth 可能为0
        if (targetImage === null || targetImage === undefined)
            return
        if (targetImage.paintedWidth > 0 && imageInfo.width < targetImage.width && imageInfo.height < targetImage.height) {
            targetImage.scale = imageInfo.width / targetImage.paintedWidth / (Window.window.width / image.width);
        } else {
            targetImage.scale = 1;
        }
    }

    function reset() {
        if (targetImage) {
            // 匹配缩放处理，对于动图，首次加载传入的 paintedWidth 可能为0
            if (targetImage.paintedWidth > 0 && imageInfo.width < targetImage.width && imageInfo.height < targetImage.height) {
                targetImage.scale = imageInfo.width / targetImage.paintedWidth;
            } else {
                targetImage.scale = 1;
            }
            targetImage.rotation = 0;
        }

        // 复位图片拖拽状态，多页图将在子 Delegate 单独处理
        if (inputHandler) {
            inputHandler.reset();
        }
    }

    function updateOffset() {
        // 需要考虑缩放时的处理
        var realWidth = targetImage.paintedWidth * targetImage.scale;
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
                if (targetImage.paintedWidth > 0) {
                    inited = true;
                }
                reset();
            }
            updateOffset();
        }

        function onScaleChanged() {
            updateOffset();
        }

        enabled: undefined !== targetImage
        target: undefined === targetImage ? null : targetImage
    }

    // 用于加载大图时的延迟显示效果
    Loader {
        id: previosLoader

        property bool needBlur: false

        active: needBlur && Image.Loading === baseDelegate.status
        anchors.fill: parent
        z: parent.z + 1

        sourceComponent: Item {
            anchors.fill: parent

            Image {
                id: loadImage

                anchors.fill: parent
                // cache会缓存数据(即便Loader重新加载)，取消此设置以正确在快速旋转/切换时从正确缓存管理中读取
                cache: false
                fillMode: Image.PreserveAspectFit
                source: "image://ThumbnailLoad/" + delegate.source + "#frame_" + delegate.frameIndex
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

        // 短时间完成加载的图片内无需模糊延迟效果
        Timer {
            interval: 20
            running: Image.Loading === baseDelegate.status

            onTriggered: {
                previosLoader.needBlur = true;
            }
        }
    }

    IV.ImageInfo {
        id: imageInfo

        source: baseDelegate.source
    }
}
