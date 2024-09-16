// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV
import "./Dialog"

Control {
    id: thumbnailView

    // 除ListView外其它按键的占用宽度
    property int btnContentWidth: switchArrowLayout.width + leftRowLayout.width + rightRowLayout.width + deleteButton.width
    // 当前是否处于文件删除状态，防止快速点击导致重复触发
    property bool imageDeleting: false
    property bool imageIsNull: null === targetImage

    // 用于外部获取当前缩略图栏内容的长度，用于布局, 10px为焦点缩略图不在ListView中的边框像素宽度(radius = 4 * 1.25)
    property int listContentWidth: bottomthumbnaillistView.contentWidth + 10
    property Image targetImage

    function deleteCurrentImage() {
        thumbnailView.imageDeleting = true;
        var trashFile = IV.GControl.currentSource;
        if (!fileTrashHelper.fileCanTrash(trashFile)) {
            // 无法移动到回收站，显示删除提示对话框
            removeDialogLoader.active = true;
        } else {
            deleteCurrentImageImpl(false);
        }
    }

    // 实际移除文件操作， directDelete：是否直接删除文件而不是移动到回收站(部分文件系统不支持)
    function deleteCurrentImageImpl(directDelete) {
        var trashFile = IV.GControl.currentSource;
        if (directDelete) {
            if (!fileTrashHelper.removeFile(trashFile)) {
                thumbnailView.imageDeleting = false;
                return;
            }
        } else {
            // 移动文件到回收站
            if (!fileTrashHelper.moveFileToTrash(trashFile)) {
                thumbnailView.imageDeleting = false;
                return;
            }
        }
        thumbnailView.imageDeleting = false;
        IV.GControl.removeImage(trashFile);

        // 删除最后图片，恢复到初始界面
        if (0 === IV.GControl.imageCount) {
            // 重置监控的文件列表
            IV.FileControl.resetImageFiles();
            stackView.switchOpenImage();
        }
    }

    function next() {
        if (repeatTimer.running) {
            return;
        }
        repeatTimer.start();

        // 切换时滑动视图不响应拖拽等触屏操作
        IV.GStatus.viewInteractive = false;
        IV.GControl.nextImage();
        IV.GStatus.viewInteractive = true;
    }

    function previous() {
        if (repeatTimer.running) {
            return;
        }
        repeatTimer.start();

        // 切换时滑动视图不响应拖拽等触屏操作
        IV.GStatus.viewInteractive = false;
        IV.GControl.previousImage();
        IV.GStatus.viewInteractive = true;
    }

    Timer {
        id: repeatTimer

        // 过快切换会使显示异常，且效果不佳
        interval: 100
    }

    // 用于文件移动至回收站/删除的辅助类
    IV.FileTrashHelper {
        id: fileTrashHelper

    }

    // 删除确认对话框加载器
    Loader {
        id: removeDialogLoader

        active: false
        asynchronous: true

        sourceComponent: RemoveDialog {
            fileName: IV.FileControl.slotGetFileNameSuffix(IV.GControl.currentSource)

            onFinished: {
                if (ret) {
                    thumbnailView.deleteCurrentImageImpl(true);
                } else {
                    // 取消删除，恢复删除按钮状态
                    thumbnailView.imageDeleting = false;
                }
                // 使用后释放对话框
                removeDialogLoader.active = false;
            }
        }
    }

    Binding {
        delayed: true
        property: "thumbnailVaildWidth"
        target: IV.GStatus
        value: window.width - 20 - btnContentWidth
    }

    Row {
        id: switchArrowLayout

        leftPadding: 10
        spacing: 10

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }

        IconButton {
            id: previousButton

            ToolTip.delay: 500
            ToolTip.text: qsTr("Previous")
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            enabled: IV.GControl.hasPreviousImage
            height: 50
            icon.name: "icon_previous"
            width: 50

            onClicked: {
                previous();
            }

            Shortcut {
                sequence: "Left"

                onActivated: {
                    previous();
                }
            }
        }

        IconButton {
            id: nextButton

            ToolTip.delay: 500
            ToolTip.text: qsTr("Next")
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            enabled: IV.GControl.hasNextImage
            height: 50
            icon.name: "icon_next"
            width: 50

            onClicked: next()

            Shortcut {
                sequence: "Right"

                onActivated: {
                    next();
                }
            }
        }
    }

    Row {
        id: leftRowLayout

        leftPadding: 40
        rightPadding: 20
        spacing: 10

        anchors {
            left: switchArrowLayout.right
            verticalCenter: parent.verticalCenter
        }

        IconButton {
            id: fitImageButton

            ToolTip.delay: 500
            ToolTip.text: qsTr("Original size")
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            anchors.leftMargin: 30
            enabled: !imageIsNull
            height: 50
            icon.name: "icon_11"
            width: 50

            onClicked: {
                imageViewer.fitImage();
                imageViewer.recalculateLiveText();
            }
        }

        IconButton {
            id: fitWindowButton

            ToolTip.delay: 500
            ToolTip.text: qsTr("Fit to window")
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            enabled: !imageIsNull
            height: 50
            icon.name: "icon_self-adaption"
            width: 50

            onClicked: {
                imageViewer.fitWindow();
                imageViewer.recalculateLiveText();
            }
        }

        IconButton {
            id: rotateButton

            ToolTip.delay: 500
            ToolTip.text: qsTr("Rotate")
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            enabled: !imageIsNull && IV.FileControl.isRotatable(IV.GControl.currentSource)
            height: 50
            icon.name: "icon_rotate"
            width: 50

            onClicked: {
                imageViewer.rotateImage(-90);
            }
        }
    }

    ListView {
        id: bottomthumbnaillistView

        property bool lastIsMultiImage: false

        // 重新定位图片位置
        function rePositionView(force) {
            // 特殊处理，防止默认显示首个缩略图时采用Center的策略会被遮挡部分
            if (force) {
                // 不再默认居中，允许在范围内切换
                // 尽可能将高亮缩略图显示在列表中
                positionViewAtIndex(currentIndex, ListView.Center);
            } else if (0 === currentIndex) {
                positionViewAtBeginning();
            } else if (currentIndex === count - 1) {
                positionViewAtEnd();
            }
        }

        cacheBuffer: 60
        clip: true
        focus: true
        height: thumbnailView.height + 10
        highlightFollowsCurrentItem: true
        // 禁用ListView自带的动画效果，但仍需要 highlight 机制使图片切换后显示在可见范围
        highlightMoveDuration: 200
        highlightMoveVelocity: -1
        // 使用范围模式，允许高亮缩略图在 preferredHighlightBegin ~ End 的范围外，使缩略图填充空白区域
        highlightRangeMode: ListView.ApplyRange
        model: IV.GControl.globalModel
        orientation: Qt.Horizontal
        preferredHighlightBegin: width / 2
        preferredHighlightEnd: width / 2
        spacing: 4
        width: thumbnailView.width - thumbnailView.btnContentWidth

        // 用于图片变更，缩略图跳变(非左右侧图片)切换时的动画效果
        Behavior on currentIndex {
            id: indexChangeBehavior

            ParallelAnimation {
                onRunningChanged: {
                    // 进入退出时均捕获当前列表显示状态
                    fadeOutEffect.scheduleUpdate();
                    if (running) {
                        fadeOutEffect.visible = true;
                    } else {
                        fadeOutEffect.visible = false;
                    }
                }

                // 淡出
                NumberAnimation {
                    duration: IV.GStatus.animationDefaultDuration
                    easing.type: Easing.OutExpo
                    from: 1
                    property: "opacity"
                    target: fadeOutEffect
                    to: 0
                }

                // 淡入
                NumberAnimation {
                    duration: IV.GStatus.animationDefaultDuration
                    easing.type: Easing.OutExpo
                    from: 0
                    property: "opacity"
                    target: bottomthumbnaillistView
                    to: 1
                }
            }
        }
        delegate: Loader {
            id: thumbnailItemLoader

            property alias frameCount: imageInfo.frameCount
            property url imageSource: model.imageUrl

            active: true
            asynchronous: true
            // NOTE:需设置默认的 Item 大小，以便于 ListView 计算 contentWidth
            // 防止 positionViewAtIndex() 时 Loader 加载，contentWidth 变化
            // 导致定位异常，同时 Delegate 使用 state 切换控件宽度
            width: Loader.Ready === status ? item.width : 30

            onActiveChanged: {
                if (active && imageInfo.delegateSource) {
                    setSource(imageInfo.delegateSource, {
                            "source": thumbnailItemLoader.imageSource
                        });
                }
            }

            IV.ImageInfo {
                id: imageInfo

                property url delegateSource
                property bool isCurrentItem: thumbnailItemLoader.ListView.isCurrentItem

                function checkDelegateSource() {
                    if (IV.ImageInfo.Ready !== status && IV.ImageInfo.Error !== status) {
                        return;
                    }
                    if (IV.Types.MultiImage === type && isCurrentItem) {
                        delegateSource = "qrc:/qml/ThumbnailDelegate/MultiThumnailDelegate.qml";
                    } else {
                        delegateSource = "qrc:/qml/ThumbnailDelegate/NormalThumbnailDelegate.qml";
                    }
                }

                source: thumbnailItemLoader.imageSource

                onDelegateSourceChanged: {
                    if (thumbnailItemLoader.active && delegateSource) {
                        setSource(delegateSource, {
                                "source": thumbnailItemLoader.imageSource
                            });
                    }
                }

                // 图片被删除、替换，重设当前图片组件
                onInfoChanged: {
                    checkDelegateSource();
                    var temp = delegateSource;
                    delegateSource = "";
                    delegateSource = temp;
                }
                onIsCurrentItemChanged: {
                    checkDelegateSource();

                    // 切换图片涉及多页图时，由于列表内容宽度变更，焦点item定位异常，延迟定位
                    if (IV.Types.MultiImage === type) {
                        bottomthumbnaillistView.lastIsMultiImage = true;
                        delayUpdateTimer.start();
                    } else if (bottomthumbnaillistView.lastIsMultiImage) {
                        delayUpdateTimer.start();
                        bottomthumbnaillistView.lastIsMultiImage = false;
                    }
                }
                onStatusChanged: checkDelegateSource()
            }
        }
        footer: Rectangle {
            width: 5
        }

        // 添加两组空的表头表尾用于占位，防止在边界的高亮缩略图被遮挡, 5px为不在ListView中维护的焦点缩略图边框的宽度 radius = 4 * 1.25
        header: Rectangle {
            width: 5
        }

        Component.onCompleted: {
            bottomthumbnaillistView.currentIndex = IV.GControl.currentIndex;
            forceLayout();
            rePositionView(true);
        }

        //滑动联动主视图
        onCurrentIndexChanged: {
            if (currentItem) {
                currentItem.forceActiveFocus();
            }

            // 仅在边缘缩略图时进行二次定位
            if (0 === currentIndex || currentIndex === (count - 1)) {
                delayUpdateTimer.start();
            }
        }

        anchors {
            left: leftRowLayout.right
            right: rightRowLayout.left
            verticalCenter: parent.verticalCenter
        }

        Connections {
            function onCurrentIndexChanged() {
                // 切换的图片为左右两侧时，不触发跳变动画
                var disableBehavior = Boolean(1 === Math.abs(IV.GControl.currentIndex - bottomthumbnaillistView.currentIndex));
                if (disableBehavior) {
                    indexChangeBehavior.enabled = false;
                }
                bottomthumbnaillistView.currentIndex = IV.GControl.currentIndex;
                if (disableBehavior) {
                    indexChangeBehavior.enabled = true;
                }
            }

            target: IV.GControl
        }

        Connections {
            function onFullScreenAnimatingChanged() {
                // 动画结束时处理
                if (!IV.GStatus.fullScreenAnimating) {
                    // 当缩放界面时，缩略图栏重新进行了布局计算，导致高亮缩略图可能不居中
                    if (0 == bottomthumbnaillistView.currentIndex) {
                        bottomthumbnaillistView.positionViewAtBeginning();
                    } else {
                        // 尽可能将高亮缩略图显示在列表中
                        bottomthumbnaillistView.positionViewAtIndex(bottomthumbnaillistView.currentIndex, ListView.Center);
                    }
                }
            }

            target: IV.GStatus
        }

        Timer {
            id: delayUpdateTimer

            interval: 100
            repeat: false

            onTriggered: {
                bottomthumbnaillistView.forceLayout();
                bottomthumbnaillistView.rePositionView(false);
            }
        }
    }

    // 捕获列表Item，用于跳变切换图片时淡入淡出效果
    ShaderEffectSource {
        id: fadeOutEffect

        anchors.fill: bottomthumbnaillistView
        hideSource: false
        // 不自动刷新，使用 scheduleUpdate() 刷新显示状态
        live: false
        recursive: false
        smooth: true
        sourceItem: bottomthumbnaillistView
        visible: true
        z: 1
    }

    Row {
        id: rightRowLayout

        leftPadding: 20
        rightPadding: 20
        spacing: 10

        anchors {
            right: deleteButton.left
            verticalCenter: parent.verticalCenter
        }

        IconButton {
            id: ocrButton

            ToolTip.delay: 500
            ToolTip.text: qsTr("Extract text")
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            enabled: IV.FileControl.isCanSupportOcr(IV.GControl.currentSource) && !imageIsNull
            height: 50
            icon.name: "icon_character_recognition"
            width: 50

            onClicked: {
                IV.GControl.submitImageChangeImmediately();
                IV.FileControl.ocrImage(IV.GControl.currentSource, IV.GControl.currentFrameIndex);
            }
        }
    }

    IconButton {
        id: deleteButton

        ToolTip.delay: 500
        ToolTip.text: qsTr("Delete")
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        enabled: !thumbnailView.imageDeleting && IV.FileControl.isCanDelete(IV.GControl.currentSource)
        height: 50
        icon.color: enabled ? "red" : "ffffff"
        icon.name: "icon_delete"
        icon.source: "qrc:/res/dcc_delete_36px.svg"
        width: 50

        onClicked: deleteCurrentImage()

        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: parent.verticalCenter
        }
    }
}
