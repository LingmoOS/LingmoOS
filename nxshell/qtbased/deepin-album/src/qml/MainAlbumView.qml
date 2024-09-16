// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import org.deepin.dtk 1.0

import org.deepin.album 1.0 as Album

import "./ThumbnailImageView"
import "./Control"
import "./SideBar"
import "./PopProgress"
import "./Control/Animation"

FadeInoutAnimation {
    anchors.fill: parent
    property int lastWidth: 0

    //rename窗口
    NewAlbumDialog {
        id: newAlbum

        Component.onCompleted: {
            GStatus.loading = false
        }
    }
    //delete窗口
    DeleteDialog {
        id: deleteDialog
    }

    // 侧边导航栏
    Sidebar{
        id : leftSidebar
        width: visible ? GStatus.sideBarWidth : 0
        anchors {
            top: parent.top
            topMargin: 19 + (titleAlubmRect.visible ? 0 : titleAlubmRect.height)
            bottom: parent.bottom
        }

        visible: true
        z: thumbnailImage.z + 1

        Component.onCompleted: {
            x =  parent.width <= GStatus.needHideSideBarWidth ? -GStatus.sideBarWidth : 0
        }
    }

    Binding {
        target: leftBgArea
        property: "visible"
        value: leftSidebar.x === 0
    }

    Binding {
        target: leftBgArea
        property: "width"
        value: leftSidebar.x + GStatus.sideBarWidth
    }

    // 侧边栏跟随窗口尺寸展开/收起
    onWidthChanged: {
        if (!GStatus.sideBarIsVisible)
            return

        if (width <= GStatus.needHideSideBarWidth) {
            if (leftSidebar.x === 0 && (lastWidth > width)) {
                hideSliderAnimation.start()
            }
        } else {
            if (leftSidebar.x < 0 && (lastWidth < width)) {
                showSliderAnimation.start()
            }
        }

        lastWidth = width
    }

    Connections {
        target: titleAlubmRect
        function onShowHideSideBar(bShow) {
            if (bShow) {
                showSliderAnimation.start()
                GStatus.sideBarIsVisible = true
            }
            else {
                hideSliderAnimation.start()
                GStatus.sideBarIsVisible = false
            }
        }
    }

    Connections {
        target: titleAlubmRect
        function onSigDeleteClicked() {
            deleteDialog.setDisplay(Album.Types.TrashSel, GStatus.selectedPaths.length)
            deleteDialog.show()
        }
    }

    Connections {
        target: titleAlubmRect
        function onShowNewAlbumDialog() {
            var x = parent.mapToGlobal(0, 0).x + parent.width / 2 - 190
            var y = parent.mapToGlobal(0, 0).y + parent.height / 2 - 89
            newAlbum.setX(x)
            newAlbum.setY(y)
            newAlbum.setNormalEdit()
            newAlbum.isChangeView = true
            newAlbum.show()
        }
    }

    //左右按钮隐藏动画
    NumberAnimation {
        id :hideSliderAnimation
        target: leftSidebar
        from: leftSidebar.x
        to: -GStatus.sideBarWidth
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }
    //左右按钮隐藏动画
    NumberAnimation {
        id :showSliderAnimation
        target: leftSidebar
        from: leftSidebar.x
        to: 0
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    ThumbnailImage{
        id: thumbnailImage
        clip: true
        anchors {
            top: parent.top
            left: leftSidebar.right
            leftMargin: 0
            topMargin: titleAlubmRect.visible ? 0 : titleAlubmRect.height
        }
        width: parent.width - leftSidebar.x - GStatus.sideBarWidth
        height: window.height - titleAlubmRect.height
    }

    StatusBar {
        id: statusBar
        anchors {
            bottom: parent.bottom
            left: leftSidebar.right
        }
//        width: leftSidebar.x == 0 ? parent.width - leftSidebar.width : window.width
        width: parent.width - leftSidebar.x - GStatus.sideBarWidth
        height: GStatus.statusBarHeight

        onSliderValueChanged: {
            GStatus.thumbnailSizeLevel = sliderValue
            FileControl.setConfigValue("", "album-zoomratio", sliderValue)
        }

        Component.onCompleted: {
            var oldSliderValue = Number(FileControl.getConfigValue("", "album-zoomratio", 4))
            setSliderWidgetValue(oldSliderValue)
            GStatus.thumbnailSizeLevel = oldSliderValue
        }
    }

    //标准弹出式进度条窗口
    StandardProgressDialog {
        id: idStandardProgressDialog
        z: leftSidebar.z + 1
    }

    //拖拽导入
    DropArea {
        anchors.fill: parent

        onDropped: (drop)=> {
            if(GStatus.currentViewType === Album.Types.ViewCustomAlbum && albumControl.isCustomAlbum(GStatus.currentCustomAlbumUId)) {
                var albumPaths = albumControl.getAlbumPaths(GStatus.currentCustomAlbumUId)
                var urls = []
                for (var i = 0; i < drop.urls.length; i++) {
                    urls.push(drop.urls[i])
                }
                if (!albumControl.checkRepeatUrls(albumPaths, urls)) {
                    albumControl.importAllImagesAndVideosUrl(drop.urls, GStatus.currentCustomAlbumUId, false)
                    albumControl.addCustomAlbumInfos(GStatus.currentCustomAlbumUId,drop.urls)
                }
            } else {
                albumControl.importAllImagesAndVideosUrl(drop.urls, GStatus.currentCustomAlbumUId, true)
            }
        }

        onEntered: (drag)=> {
            if(drag.hasUrls) {
                var urls = drag.urls
                if(FileControl.checkMimeUrls(urls)) {
                    if (GStatus.currentViewType === Album.Types.ViewCustomAlbum) {
                        // 仅自定义相册允许拖拽导入
                        drag.accepted = albumControl.isCustomAlbum(GStatus.currentCustomAlbumUId)
                    } else if (GStatus.currentViewType === Album.Types.ViewImport
                               || GStatus.currentViewType === Album.Types.ViewCollecttion
                               || GStatus.currentViewType === Album.Types.ViewHaveImported) {
                        drag.accepted = true
                    } else {
                        drag.accepted = false
                    }
                } else {
                    drag.accepted = false
                }
            } else {
                drag.accepted = false
            }
        }
    }

    Connections {
        // 关联外部通过 DBus 等方式触发调用相册
        function onOpenImageFile(fileName) {
            var paths = []
            paths.push(fileName)
            openAndImportImages(paths)
        }

        target: FileControl
    }

    Connections {
        target: albumControl
        // 接收外部应用打开信号
        function onSigOpenImageFromFiles(paths) {
            openAndImportImages(paths)
        }

        //收到导入开始消息
        onSigImportStart: {
            idStandardProgressDialog.clear()
            idStandardProgressDialog.setTitle(qsTr("Importing..."))
            var prevS = qsTr("Imported:")
            var suffixS = "0"
            idStandardProgressDialog.setContent(prevS + suffixS)
            idStandardProgressDialog.setProgress(0, 100)
            idStandardProgressDialog.show()
            leftSidebar.enabled = false
            thumbnailImage.enabled = false
            titleAlubmRect.enabled = false
        }

        //收到导入进度消息
        onSigImportProgress: {
            var prevS = qsTr("Imported:")
            var suffixS = qsTr("%1/%2").arg(value).arg(max)
            idStandardProgressDialog.setContent(prevS + suffixS)
            idStandardProgressDialog.setProgress(value, max)
        }

        //收到导入完成消息
        onSigImportFinished: {
            idStandardProgressDialog.close()
            leftSidebar.enabled = true
            thumbnailImage.enabled = true
            titleAlubmRect.enabled = true
            DTK.sendMessage(stackControl, qsTr("Import successful"), "notify_checked")
        }

        //收到导入完成消息
        onSigRepeatUrls: {
            idStandardProgressDialog.close()
            leftSidebar.enabled = true
            thumbnailImage.enabled = true
            titleAlubmRect.enabled = true
            if (urls.length === 0)
                DTK.sendMessage(stackControl, qsTr("Import failed"), "warning")
        }
    }

    //打开看图查看图片
    function openAndImportImages(paths) {
        if (paths.length === 0)
            return

        var tempPath = ""

        if (paths.length > 0)
            tempPath = paths[0]

        if(paths.length > 0 && tempPath !== ""){
            if (FileControl.isVideo(tempPath)){
                window.title = ""
                //TODO： V20右键打开视频文件，不会调影院播放，因此该功能同步V20，若V23要求使用影院播放，放开下面代码即可
                //albumControl.openDeepinMovie(tempPath)
            } else if (FileControl.isImage(tempPath)){
                GControl.setImageFiles(paths, tempPath)
                FileControl.resetImageFiles(paths);
                mainStack.switchImageView()
                GStatus.stackControlCurrent = 1
            }
        }

        // 若在文管菜单使用相册打开图片文件，并且数据库中未导入该图片，应该将选择的图片导入相册中
        if (tempPath !== "" && !albumControl.checkRepeatUrls(albumControl.getAllUrlPaths(), paths, false)) {
            albumControl.importAllImagesAndVideos(paths)
        }
    }

    Component.onCompleted: {
        var tempPaths = []
        tempPaths = FileControl.parseCommandlineGetPaths()

        openAndImportImages(tempPaths)
    }
}
