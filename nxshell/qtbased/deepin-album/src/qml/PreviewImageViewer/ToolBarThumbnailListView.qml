// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0

Item {
    property int currentIndex: 0
    // 用于外部获取当前缩略图栏内容的长度，用于布局
    property alias listContentWidth: bottomthumbnaillistView.contentWidth

    onCurrentIndexChanged: {
        bottomthumbnaillistView.currentIndex = currentIndex
        bottomthumbnaillistView.forceActiveFocus()
    }

    function rotateImage( x ){
        bottomthumbnaillistView.currentItem.rotation=bottomthumbnaillistView.currentItem.rotation +x
    }

    function deleteCurrentImage(){
        var tmpPath = source
        var tmpPaths = []
        tmpPaths.push(tmpPath)
        if (mainView.sourcePaths.length - 1 > bottomthumbnaillistView.currentIndex) {
            var tempPathIndex = bottomthumbnaillistView.currentIndex
            //需要保存临时变量，重置后赋值
            imageViewer.sourcePaths = FileControl.removeList(sourcePaths,tempPathIndex)
            imageViewer.swipeIndex=tempPathIndex
            if (!FileControl.isAlbum())
                FileControl.deleteImagePath(tmpPath)
            else {
                albumControl.insertTrash(tmpPaths)
            }
        }else if(mainView.sourcePaths.length - 1 == 0){
            stackView.currentWidgetIndex=0
            window.title=""
            if (!FileControl.isAlbum())
                FileControl.deleteImagePath(imageViewer.sourcePaths[0])
            else {
                GStatus.stackControlCurrent = 0
                albumControl.insertTrash(tmpPaths)
            }
            imageViewer.sourcePaths=FileControl.removeList(sourcePaths,0)

        }else{
            bottomthumbnaillistView.currentIndex--
            imageViewer.source = imageViewer.sourcePaths[bottomthumbnaillistView.currentIndex]
            if (!FileControl.isAlbum())
                FileControl.deleteImagePath(sourcePaths[bottomthumbnaillistView.currentIndex+1])
            else
                albumControl.insertTrash(tmpPaths)
            imageViewer.sourcePaths = FileControl.removeList(imageViewer.sourcePaths,bottomthumbnaillistView.currentIndex+1)
        }
    }

    function previous (){
        // 判断是否为多页图,多页图只进行页面替换
        if (imageViewer.currentIsMultiImage) {
            if (imageViewer.frameIndex != 0) {
                imageViewer.frameIndex--
                return
            }
        }

        if (bottomthumbnaillistView.currentIndex > 0) {
            bottomthumbnaillistView.currentIndex--
            source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
            imageViewer.index = currentIndex

            // 向前移动的图像需要特殊判断，若为多页图，调整显示最后一张图
            if (FileControl.isMultiImage(source)) {
                imageViewer.frameIndex = FileControl.getImageCount(source) - 1;
            }
            bottomthumbnaillistView.forceActiveFocus()
        }
    }

    function next (){
        // 判断是否为多页图,多页图只进行页面替换
        if (imageViewer.currentIsMultiImage) {
            if (imageViewer.frameIndex < imageViewer.frameCount - 1) {
                imageViewer.frameIndex++
                return
            }
        }

        if (mainView.sourcePaths.length - 1 > bottomthumbnaillistView.currentIndex) {
            bottomthumbnaillistView.currentIndex++
            source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
            imageViewer.index = currentIndex
            bottomthumbnaillistView.forceActiveFocus()
        }
    }
    IconButton {
        id:backAlbum
        width: FileControl.isAlbum() ? 50 : 0
        height:  FileControl.isAlbum() ? 50 : 0
        icon.name: "back_album"
        icon.width: 36
        icon.height: 36
        anchors.left: parent.left
        anchors.leftMargin: 15
        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        onClicked: {
            showNormal()
            GStatus.stackControlCurrent = 0
            window.title = ""
        }

        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Back to Album")

        Shortcut {
            enabled: backAlbum.visible
                     && GStatus.stackControlCurrent === 1
                     && FileControl.isAlbum()
                     && window.visibility !== Window.FullScreen
            sequence: "Esc"
            onActivated: {
                showNormal()
                GStatus.stackControlCurrent = 0
                window.title = ""
            }
        }
    }
    IconButton {
        id: previousButton
        enabled: currentIndex > 0
                 || imageViewer.frameIndex > 0
        anchors.left: FileControl.isAlbum() ? backAlbum.right :parent.left
        anchors.leftMargin: FileControl.isAlbum() ? 30 : 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        width:50
        height:50
        icon.name: "icon_previous"
        onClicked: {
            previous();
        }

        Shortcut{
            enabled: stackView.enabled
            sequence: "Left"
            onActivated: previous();
        }

        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Previous")

    }
    IconButton {
        id: nextButton
        enabled: currentIndex < mainView.sourcePaths.length - 1
                 || imageViewer.frameIndex < imageViewer.frameCount - 1
        anchors.left: previousButton.right
        anchors.leftMargin: 10

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        width:50
        height:50
        icon.name:"icon_next"
        onClicked: {
            next();
        }
        Shortcut{
            enabled: stackView.enabled
            sequence: "Right"
            onActivated: next();
        }
        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Next")
    }


        IconButton {
            id: fitImageButton
            anchors.left: nextButton.right
            anchors.leftMargin: FileControl.isAlbum() ? 10 : 40

            anchors.top: parent.top
            anchors.topMargin: (parent.height - height) / 2
            width:50
            height:50
            icon.name:"icon_11"
            enabled:!CodeImage.imageIsNull(imageViewer.source)
            onClicked: {
                imageViewer.fitImage()
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Original size")
        }
        IconButton {
            id: fitWindowButton
            anchors.left: fitImageButton.right
            anchors.leftMargin:10

            anchors.top: parent.top
            anchors.topMargin: (parent.height - height) / 2

            width:50
            height:50
            icon.name:"icon_self-adaption"
            enabled:!CodeImage.imageIsNull(imageViewer.source)
            onClicked: {
                imageViewer.fitWindow()
            }

            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Fit to window")
        }

        IconButton {
            id: rotateButton
            anchors.left: fitWindowButton.right
            anchors.leftMargin:10

            anchors.top: parent.top
            anchors.topMargin: (parent.height - height) / 2

            width:50
            height:50
            icon.name:"icon_rotate"
            enabled:!CodeImage.imageIsNull(imageViewer.source) && FileControl.isRotatable(imageViewer.source)
            onClicked: {
                imageViewer.rotateImage(-90)

            // 动态刷新导航区域图片内容，同时可在imageviewer的sourceChanged中隐藏导航区域
            // (因导航区域图片source绑定到imageviewer的source属性)
                imageViewer.source = ""
                imageViewer.source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Rotate")
        }

        IconButton {
            id: collectionButton
            property bool canFavorite: !albumControl.photoHaveFavorited(source, GStatus.bRefreshFavoriteIconFlag)
            width: FileControl.isAlbum() ? 50 : 0
            height:  FileControl.isAlbum() ? 50 : 0
            visible: FileControl.isAlbum()
            icon.name: canFavorite ? "toolbar-collection" : "toolbar-collection2"
            icon.width:36
            icon.height:36

            anchors.left: rotateButton.right
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: (parent.height - height) / 2

            onClicked: {
                var paths = []
                paths.push(source)
                if (canFavorite)
                    albumControl.insertIntoAlbum(0, paths)
                else {
                    albumControl.removeFromAlbum(0, paths)
                }

                GStatus.bRefreshFavoriteIconFlag = !GStatus.bRefreshFavoriteIconFlag
            }

            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: canFavorite ? qsTr("Favorite") : qsTr("Unfavorite")
        }


    ListView {
        id: bottomthumbnaillistView

        // 使用范围模式，允许高亮缩略图在preferredHighlightBegin~End的范围外，使缩略图填充空白区域
        highlightRangeMode: ListView.ApplyRange
        highlightFollowsCurrentItem: true

        preferredHighlightBegin: width / 2 - 25
        preferredHighlightEnd: width / 2 + 25

        anchors.left: FileControl.isAlbum() ? collectionButton.right : rotateButton.right
        anchors.leftMargin: 10

        anchors.right: ocrButton.left
        anchors.rightMargin: 10

        anchors.top: parent.top
        anchors.topMargin: -5

        height: parent.height + 10
        width: parent.width - deleteButton.width - 60

        clip: true
        spacing: 10
        focus: true

        //滑动联动主视图
        onCurrentIndexChanged: {
            mainView.currentIndex = currentIndex
            source = mainView.sourcePaths[currentIndex]
            if (currentItem) {
                currentItem.forceActiveFocus()
            }

            // 特殊处理，防止默认显示首个缩略图时采用Center的策略会被遮挡部分
            if (0 == currentIndex) {
                positionViewAtBeginning()
            } else {
                // 尽可能将高亮缩略图显示在列表中
                positionViewAtIndex(currentIndex, ListView.Center)
            }
        }

        Connections {
            target: imageViewer
            onSwipeIndexChanged: {
                var imageSwipeIndex = imageViewer.swipeIndex
                if (currentIndex - imageSwipeIndex == 1) {
                    // 向前切换当通过拖动等方式时，调整多页图索引为最后一张图片
                    var curSource = sourcePaths[imageSwipeIndex]
                    if (FileControl.isMultiImage(curSource)) {
                        imageViewer.frameIndex = FileControl.getImageCount(curSource) - 1
                    }
                } else {
                    // 其它情况均设置为首张图片
                    imageViewer.frameIndex = 0
                }

                bottomthumbnaillistView.currentIndex = imageSwipeIndex
                currentIndex= imageSwipeIndex
                bottomthumbnaillistView.forceActiveFocus()
            }

            onIsFullNormalSwitchStateChanged: {
                // 当缩放界面时，缩略图栏重新进行了布局计算，导致高亮缩略图可能不居中
                if (0 == bottomthumbnaillistView.currentIndex) {
                    bottomthumbnaillistView.positionViewAtBeginning()
                } else {
                    // 尽可能将高亮缩略图显示在列表中
                    bottomthumbnaillistView.positionViewAtIndex(bottomthumbnaillistView.currentIndex, ListView.Center)
                }
            }

            // 接收当前视图旋转角度变更信号
            onCurrentRotateChanged: {
                if (bottomthumbnaillistView.currentItem) {
                    // 计算旋转角度，限制在旋转梯度为90度，以45度为分界点
                    var rotateAngle = imageViewer.currentRotate;
                    // 区分正反旋转方向
                    var isClockWise = rotateAngle > 0
                    // 计算绝对角度值
                    rotateAngle = Math.floor((Math.abs(rotateAngle) + 45) / 90) * 90;

                    // 设置当前展示的图片的旋转方向，仅在90度方向旋转，不会跟随旋转角度(特指在触摸状态下)
                    bottomthumbnaillistView.currentItem.rotation = isClockWise ? rotateAngle : -rotateAngle
                }
            }
        }

        orientation: Qt.Horizontal

        cacheBuffer: 400
        model: mainView.sourcePaths.length
        delegate: ListViewDelegate {
        }

         // 添加两组空的表头表尾用于占位，防止在边界的高亮缩略图被遮挡
        header: Rectangle {
            width: 10
        }

        footer: Rectangle {
            width: 10
        }

        Behavior on x {
            NumberAnimation {
                duration: 50
                easing.type: Easing.OutQuint
            }
        }

        Behavior on y {
            NumberAnimation {
                duration: 50
                easing.type: Easing.OutQuint
            }
        }
    }

    IconButton {
        id :ocrButton
        width:50
        height:50
        icon.name:"icon_character_recognition"
        anchors.right: deleteButton.left
        anchors.rightMargin: 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2
        enabled: FileControl.isCanSupportOcr(source) && !CodeImage.imageIsNull(source)
        onClicked: {
            FileControl.ocrImage(source)
        }
        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Extract text")
    }

    IconButton {
        id: deleteButton
        width:50
        height:50
        icon.name: "icon_delete"

        anchors.right: parent.right
        anchors.rightMargin: 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        icon.source: "qrc:/res/dcc_delete_36px.svg"
        icon.color: enabled ? "red" :"ffffff"
        onClicked: {
            deleteCurrentImage()
        }
        //        visible: FileControl.isCanDelete(source) ? true :false

        enabled: FileControl.isCanDelete(source) ? true :false

        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Delete")

    }
}
