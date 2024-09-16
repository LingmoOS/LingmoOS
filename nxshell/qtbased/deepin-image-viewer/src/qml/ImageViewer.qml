// SPDX-FileCopyrightText: 2023-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV
import "./ImageDelegate"
import "./LiveText"
import "./InformationDialog"
import "./Utils"

Item {
    id: imageViewer

    // 记录图像缩放，用于在窗口缩放时，根据前后窗口变化保持图片缩放比例
    property bool enableChangeDisplay: true
    property real lastDisplayScaleWidth: 0
    // Image 类型的对象，空图片、错误图片、消失图片等异常为 null
    property alias targetImage: view.currentImage

    // Note: 对于SVG、动图等特殊类型图片，使用 targeImage 获取的图片 sourceSize 存在差异，
    // 可能为零或导致缩放模糊，调整为使用从文件中读取的原始大小计算。
    // 图片旋转后同样会交换宽度和高度，更新缓存的图片源宽高信息
    property alias targetImageInfo: currentImageInfo
    property bool targetImageReady: (null !== view.currentImage) && (Image.Ready === view.currentImage.status)

    // 退出全屏展示图片
    function escBack() {
        IV.GStatus.showImageInfo = false;
        showNormal();
        showfullAnimation.start();
    }

    function exitLiveText() {
        ltw.exitLiveText();
    }

    function fitImage() {
        if (targetImageReady) {
            // 按图片原始大小执行缩放
            imageAnimation.scaleAnime(targetImageInfo.width / targetImage.paintedWidth);
        }
    }

    function fitWindow() {
        // 默认状态的图片即适应窗口大小(使用 Image.PreserveAspectFit)
        if (targetImageReady) {
            imageAnimation.scaleAnime(1.0);
        }
    }

    // 窗口拖拽大小变更时保持图片的显示缩放比例
    function keepImageDisplayScale() {
        if (!targetImageReady) {
            return;
        }

        // 当前缩放比例与匹配窗口的图片缩放比例比较，不一致则保持缩放比例
        if (Math.abs(targetImage.scale - 1.0) > Number.EPSILON) {
            if (0 !== lastDisplayScaleWidth) {
                // Note: 拖拽窗口时将保持 scale ，但 paintedWidth / paintedHeight 将变更
                // 因此在此处设置缩放比例时屏蔽重复设置，以保留缩放比例
                enableChangeDisplay = false;
                targetImage.scale = lastDisplayScaleWidth / targetImage.paintedWidth;
                enableChangeDisplay = true;
            } else {
                lastDisplayScaleWidth = targetImage.paintedWidth * targetImage.scale;
            }
        } else {
            // 一致则保持匹配窗口
            fitWindow();
        }
    }

    function recalculateLiveText() {
        if (targetImageReady && IV.Types.DynamicImage !== currentImageInfo.type) {
            exitLiveText();
            startLiveText();
        }
    }

    function rotateImage(angle) {
        if (targetImageReady && !rotateDelay.running) {
            rotateDelay.start();
            IV.GControl.currentRotation += angle;
        }
    }

    // 触发全屏展示图片
    function showPanelFullScreen() {
        ltw.exitLiveText();
        IV.GStatus.showImageInfo = false;
        showFullScreen();
        view.contentItem.forceActiveFocus();
        showfullAnimation.start();
    }

    function showScaleFloatLabel() {
        // 不存在的图片不弹出缩放提示框
        if (!targetImageReady) {
            return;
        }

        // 图片实际缩放比值 绘制像素宽度 / 图片原始像素宽度
        var readableScale = targetImage.paintedWidth * targetImage.scale / targetImageInfo.width * 100;
        if (readableScale.toFixed(0) > 2000 && readableScale.toFixed(0) <= 3000) {
            floatLabel.displayStr = "2000%";
        } else if (readableScale.toFixed(0) < 2 && readableScale.toFixed(0) >= 0) {
            floatLabel.displayStr = "2%";
        } else if (readableScale.toFixed(0) >= 2 && readableScale.toFixed(0) <= 2000) {
            floatLabel.displayStr = readableScale.toFixed(0) + "%";
        }
        floatLabel.visible = true;
    }

    function startLiveText() {
        ltw.startLiveTextAnalyze();
    }

    onHeightChanged: keepImageDisplayScale()
    onTargetImageChanged: {
        // 部分多页图 targetImage 变更时不会重复触发 targetImageReady ，在此处进行备用的判断
        if (IV.Types.DynamicImage !== currentImageInfo.type) {
            // 适配窗口
            recalculateLiveText();
        } else {
            exitLiveText();
        }
    }

    // 图片状态变更时触发
    onTargetImageReadyChanged: {
        if (IV.Types.DynamicImage !== currentImageInfo.type) {
            // 适配窗口
            recalculateLiveText();
        }
        showScaleFloatLabel();

        // 重置保留的缩放状态
        lastDisplayScaleWidth = 0;
    }
    onWidthChanged: keepImageDisplayScale()

    Timer {
        id: rotateDelay

        interval: IV.GStatus.animationDefaultDuration + 50
    }

    // 图像动画：缩放
    ImageAnimation {
        id: imageAnimation

        targetImage: imageViewer.targetImage
    }

    Connections {
        function onPaintedHeightChanged() {
            recalculateLiveText();
        }

        function onPaintedWidthChanged() {
            recalculateLiveText();
        }

        function onScaleChanged() {
            // 图片实际缩放比值 绘制像素宽度 / 图片原始像素宽度
            var readableScale = targetImage.paintedWidth * targetImage.scale / targetImageInfo.width * 100;
            // 缩放限制在 2% ~ 2000% ，变更后再次进入此函数处理
            if (readableScale < 2) {
                targetImage.scale = targetImageInfo.width * 0.02 / targetImage.paintedWidth;
                return;
            } else if (readableScale > 2000) {
                targetImage.scale = targetImageInfo.width * 20 / targetImage.paintedWidth;
                return;
            }

            // 处于保持效果缩放状态时，保留之前的缩放比例
            if (enableChangeDisplay) {
                lastDisplayScaleWidth = targetImage.paintedWidth * targetImage.scale;
                // 显示缩放框
                showScaleFloatLabel();
            }

            // 重新文本识别
            recalculateLiveText();
        }

        function onXChanged() {
            recalculateLiveText();
        }

        function onYChanged() {
            recalculateLiveText();
        }

        enabled: targetImageReady
        ignoreUnknownSignals: true
        target: targetImage
    }

    // 触发切换全屏状态
    Connections {
        function onShowFullScreenChanged() {
            if (window.isFullScreen !== IV.GStatus.showFullScreen) {
                // 关闭详细信息窗口
                IV.GStatus.showImageInfo = false;
                IV.GStatus.showFullScreen ? showPanelFullScreen() : escBack();
            }
        }

        target: IV.GStatus
    }

    PropertyAnimation {
        id: showfullAnimation

        duration: 200
        easing.type: Easing.InExpo
        from: 0
        property: "opacity"
        target: parent.Window.window
        to: 1

        onRunningChanged: {
            IV.GStatus.fullScreenAnimating = running;
            // 动画结束时，重置缩放状态
            if (!running && targetImageReady) {
                // 匹配缩放处理
                if (targetImageInfo.height < targetImage.height) {
                    targetImage.scale = targetImageInfo.width / targetImage.paintedWidth;
                } else {
                    targetImage.scale = 1.0;
                }
            }
        }
    }

    //缩放快捷键
    Shortcut {
        sequence: "Ctrl+="

        onActivated: {
            targetImage.scale = targetImage.scale / 0.9;
        }
    }

    Shortcut {
        sequence: "Ctrl+-"

        onActivated: {
            targetImage.scale = targetImage.scale * 0.9;
        }
    }

    Shortcut {
        sequence: "Up"

        onActivated: {
            targetImage.scale = targetImage.scale / 0.9;
        }
    }

    Shortcut {
        sequence: "Down"

        onActivated: {
            targetImage.scale = targetImage.scale * 0.9;
        }
    }

    // 图片滑动视图的上层组件
    Control {
        id: viewBackground

        anchors.fill: parent
    }

    // 图片滑动视图
    PathView {
        id: view

        // 当前展示的 Image 图片对象，空图片、错误图片、消失图片等异常为 undefined
        // 此图片信息用于外部交互缩放、导航窗口等，已标识类型，使用 null !== currentImage 判断
        property Image currentImage: {
            if (view.currentItem) {
                if (view.currentItem.item) {
                    return view.currentItem.item.targetImage;
                }
            }
            return null;
        }
        // 用于限制拖拽方向(处于头尾时)
        property real previousOffset: 0

        // WARNING: 目前 ListView 组件屏蔽输入处理，窗口拖拽依赖底层的 ApplicationWindow
        // 因此不允许 ListView 的区域超过标题栏，图片缩放超过显示区域无妨。
        // 显示图片上下边界距边框 50px (标题栏宽度)，若上下间隔不一致时，进行拖拽、导航定位或需减去(间隔差/2)
        // 在全屏时无上下边框
        anchors.horizontalCenter: parent.horizontalCenter
        dragMargin: width / 2
        flickDeceleration: 500
        focus: true
        height: window.isFullScreen ? parent.height : (parent.height - (IV.GStatus.titleHeight * 2))
        // 动画过程中不允许拖拽
        interactive: !IV.GStatus.fullScreenAnimating && IV.GStatus.viewInteractive && !offsetAnimation.running
        model: IV.GControl.viewModel
        // 设置滑动视图的父组件以获取完整的OCR图片信息
        parent: viewBackground
        // PathView 的动画效果通过 Path 路径和 Item 个数及 Item 宽度共同计算
        pathItemCount: IV.GStatus.pathViewItemCount
        preferredHighlightBegin: 0.5
        preferredHighlightEnd: 0.5
        snapMode: ListView.SnapOneItem
        width: parent.width
        y: Window.window.isFullScreen ? 0 : IV.GStatus.titleHeight

        // 代理组件加载器
        delegate: ViewDelegateLoader {
        }
        Behavior on offset {
            id: offsetBehavior

            enabled: !IV.GStatus.viewFlicking

            NumberAnimation {
                id: offsetAnimation

                duration: IV.GStatus.animationDefaultDuration
                easing.type: Easing.OutExpo

                onRunningChanged: {
                    // 动画结束，触发更新同步状态
                    IV.GControl.viewModel.syncState();
                }
            }
        }

        // 注意图片路径是按照 总长 / pathItemCount 来平均计算位置的，各项间距等分
        path: Path {
            startX: 0
            startY: view.height / 2

            // 前一图片位置
            PathLine {
                x: view.width / 6
                y: view.height / 2
            }

            PathAttribute {
                name: "delegateOpacity"
                value: 0
            }

            PathAttribute {
                name: "delegateOffset"
                value: -1
            }

            // 当前图片位置
            PathLine {
                x: view.width / 2
                y: view.height / 2
            }

            PathAttribute {
                name: "delegateOpacity"
                value: 1
            }

            PathAttribute {
                name: "delegateOffset"
                value: 0
            }

            // 后一图片位置
            PathLine {
                x: view.width * 5 / 6
                y: view.height / 2
            }

            PathAttribute {
                name: "delegateOpacity"
                value: 0
            }

            PathAttribute {
                name: "delegateOffset"
                value: 1
            }

            PathLine {
                x: view.width
                y: view.height / 2
            }
        }

        Component.onCompleted: {
            // 首次进入(退出缩略图后创建)重置当前显示的索引
            IV.GStatus.viewFlicking = true;
            currentIndex = IV.GControl.viewModel.currentIndex;
            IV.GStatus.viewFlicking = false;
        }
        onCurrentIndexChanged: {
            var curIndex = view.currentIndex;
            var previousIndex = IV.GControl.viewModel.currentIndex;
            var lastIndex = view.count - 1;

            // 若索引变更通过model触发，则没有必要更新
            if (curIndex == previousIndex) {
                return;
            }

            // 特殊场景处理，到达边界后循环显示图片
            if (0 === curIndex && previousIndex === lastIndex) {
                IV.GControl.nextImage();
                return;
            }
            if (curIndex === lastIndex && 0 === previousIndex) {
                IV.GControl.previousImage();
                return;
            }

            // 当通过界面拖拽导致索引变更，需要调整多页图索引范围
            if (view.currentIndex < previousIndex) {
                IV.GControl.previousImage();
            } else if (view.currentIndex > previousIndex) {
                IV.GControl.nextImage();
            }
        }
        onMovementEnded: {
            IV.GStatus.viewFlicking = false;
        }
        onMovementStarted: {
            IV.GStatus.viewFlicking = true;
            previousOffset = offset;
        }

        Connections {
            // 模型的索引变更时(缩略图栏点击)触发图片切换的动画效果
            function onCurrentIndexChanged(index) {
                if (view.currentIndex === index) {
                    return;
                }

                /*  NOTE: 由于 PathView 循环显示的特殊性，index 递增，而 offset 递减(index + offset = count)
                    以 count = 5 为例， 在边界 (index 0->4 4->0 0->1) 场景会出现跳变现象
                    index 0 -> 1 对应的 offset 是 0 -> 4 ，实际动画会经过 index 0 4 3 2 1

                    此问题可以通过对场景特殊判断处理，但是动画过程中的 offset 不定，需要结束之前动画后再调整
                */
                if (offsetAnimation.running) {
                    var targetValue = offsetBehavior.targetValue;
                    offsetAnimation.complete();
                    IV.GStatus.viewFlicking = true;
                    view.offset = targetValue;
                    IV.GStatus.viewFlicking = false;
                }

                // 计算相对距离，调整 offset 以触发动画效果
                var distance = Math.abs(view.currentIndex - index);
                if (distance !== 1 && distance !== view.count - 1) {
                    // 动画处理
                    view.currentIndex = index;
                    return;
                }
                var lastIndex = view.count - 1;

                // 调整 offset 进行坐标偏移
                var oldOffset = view.offset;
                var newOffset = (view.count - index);
                if (view.currentIndex === 0 && 1 === index) {
                    IV.GStatus.viewFlicking = true;
                    view.offset = view.count - 0.00001;
                    IV.GStatus.viewFlicking = false;
                } else if (view.currentIndex === lastIndex && 0 === index) {
                    newOffset = 0;
                }
                view.offset = newOffset;
            }

            target: IV.GControl.viewModel
        }

        IV.PathViewRangeHandler {
            enableBackward: IV.GControl.hasNextImage
            enableForward: IV.GControl.hasPreviousImage
            target: view
        }
    }

    IV.ImageInfo {
        id: currentImageInfo

        frameIndex: IV.GControl.currentFrameIndex
        source: IV.GControl.currentSource
    }

    Connections {
        function onViewFlickingChanged() {
            if (IV.GStatus.viewFlicking) {
                ltw.exitLiveText();
            } else {
                ltw.startLiveTextAnalyze();
            }
        }

        target: IV.GStatus
    }

    // 实况文本蒙版
    LiveTextWidget {
        //live text主控件
        id: ltw

        //live text退出函数
        function exitLiveText() {
            liveTextAnalyzer.breakAnalyze();
            ltw.clearLive();
            liveTextTimer.stop();
        }

        //live text分析函数
        //缩放和切换需要重新执行此函数
        function liveTextAnalyze() {
            viewBackground.grabToImage(function (result) {
                    //截取当前控件显示
                    liveTextAnalyzer.setImage(result.image); //设置分析图片
                    liveTextAnalyzer.analyze(view.currentIndex); //执行分析（异步执行，函数会立即返回）
                    // result.saveToFile("/home/user/Desktop/viewer.png") //保存截取的图片，debug用
                });
        }

        //live text执行函数
        function runLiveText(resultCanUse, token) {
            if (resultCanUse && token == view.currentIndex) {
                //这里无视警告，就是需要js的==来进行自动类型转换
                console.debug("run live start");
                ltw.drawRect(liveTextAnalyzer.liveBlock());
                ltw.visible = true;
            }
        }

        //live text分析启动控制
        function startLiveTextAnalyze() {
            if (targetImageReady && IV.Types.DynamicImage !== currentImageInfo.type) {
                liveTextTimer.restart();
            }
        }

        anchors.fill: parent

        Component.onCompleted: {
            liveTextAnalyzer.analyzeFinished.connect(runLiveText);
        }

        // 实况文本背景遮罩
        Rectangle {
            id: liveTextBackground

            anchors.centerIn: parent
            color: "#000000" // live text高亮阴影
            opacity: 0.5
            visible: highlightTextButton.checked && highlightTextButton.visible

            onVisibleChanged: {
                var target = view.currentImage;
                if (null !== target) {
                    width = target.paintedWidth * target.scale;
                    height = target.paintedHeight * target.scale;
                }
            }
        }

        //live text分析启动延迟
        Timer {
            id: liveTextTimer

            interval: 1500
            repeat: false
            running: false

            onTriggered: {
                var supportOcr = IV.FileControl.isCanSupportOcr(IV.GControl.currentSource);
                if (supportOcr && targetImageReady && IV.Types.DynamicImage !== currentImageInfo.type) {
                    // 执行条件和OCR按钮使能条件一致
                    ltw.liveTextAnalyze();
                    running = false;
                }
            }
        }
    }

    FloatingButton {
        id: highlightTextButton

        property bool isHighlight: false

        checked: isHighlight
        height: 50
        parent: imageViewerArea
        visible: false
        width: 50
        z: ltw.z + 100

        // 高亮时不弹出工具栏栏以方便选取
        onCheckedChanged: {
            IV.GStatus.animationBlock = checked;
        }
        onClicked: {
            isHighlight = !isHighlight;
        }
        onVisibleChanged: {
            if (!visible) {
                IV.GStatus.animationBlock = false;
            }
        }

        anchors {
            bottom: parent.bottom
            bottomMargin: thumbnailViewBackGround.height + 20
            right: parent.right
            rightMargin: 100
        }

        DciIcon {
            anchors.centerIn: parent
            height: 45
            name: "icon_recognition_highlight"
            palette: DTK.makeIconPalette(parent.palette)
            width: 45
        }
    }

    //rename窗口
    ReName {
        id: renamedialog

    }

    // 右键菜单
    ViewRightMenu {
        id: rightMenu

        // 拷贝快捷键冲突：选中实况文本时，屏蔽拷贝图片的快捷键
        copyableConfig: !ltw.currentHasSelect

        // 菜单销毁后也需要发送信号，否则可能未正常送达
        Component.onDestruction: {
            IV.GStatus.showRightMenu = false;
        }
        onClosed: {
            IV.GStatus.showRightMenu = false;
            imageViewer.forceActiveFocus();
        }

        Connections {
            function onShowRightMenuChanged() {
                if (IV.GStatus.showRightMenu) {
                    rightMenu.popup(IV.CursorTool.currentCursorPos());
                    rightMenu.focus = true;

                    // 关闭详细信息弹窗
                    IV.GStatus.showImageInfo = false;
                }
            }

            target: IV.GStatus
        }
    }

    // 图片信息窗口
    Loader {
        id: infomationDig

        function show() {
            IV.GStatus.showImageInfo = true;
        }

        active: IV.GStatus.showImageInfo
        asynchronous: true

        // 图片属性信息窗口
        sourceComponent: InformationDialog {
        }
    }

    //导航窗口
    Loader {
        id: naviLoader

        // 导航窗口是否显示
        property bool expectShow: IV.GStatus.enableNavigation && (null !== targetImage) && (targetImage.scale > 1)

        height: 112
        width: 150

        sourceComponent: NavigationWidget {
            // 根据当前缩放动画预期的缩放比例调整导航窗口是否提前触发隐藏
            prefferHide: {
                if (imageAnimation.running) {
                    return imageAnimation.prefferImageScale <= 1;
                }
                return false;
            }
            targetImage: view.currentImage
            // 默认位置，窗体底部
            y: naviLoader.height + 70

            // 长时间隐藏，请求释放导航窗口
            onRequestRelease: {
                naviLoader.active = false;
            }
        }

        // 仅控制弹出显示导航窗口
        onExpectShowChanged: {
            if (expectShow) {
                active = true;
            }
        }

        anchors {
            bottom: parent.bottom
            bottomMargin: 109
            left: parent.left
            leftMargin: 15
        }
    }
}
