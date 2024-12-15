// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

import org.lingmo.album 1.0 as Album

Item {
    property bool isInTrash: GStatus.currentViewType === Album.Types.ViewRecentlyDeleted
    property bool isInDevice: GStatus.currentViewType === Album.Types.ViewDevice
    //菜单项状态检查
    property bool haveImage: false
    property bool haveVideo: false
    property bool canFullScreen: false
    property bool canSlideShow: false
    property bool canView: false
    property bool canExport: false
    property bool canCopy: false
    property bool canDelete: false
    property bool canViewPhotoInfo: false
    property bool canViewVideoInfo: false
    property bool canWallpaper: false
    property bool canFavorite: {
        GStatus.bRefreshFavoriteIconFlag
        return albumControl.canFavorite(GStatus.selectedPaths) && !isInTrash
    }
    property bool canRotate: true
    property bool canDisplayInFolder: false
    property bool canPrint: true

    property var selectedUrls: GStatus.selectedPaths

    //已选的图片状态检查
    function updateMenuItemStates() {
        haveImage = FileControl.haveImage(GStatus.selectedPaths)
        haveVideo = FileControl.haveVideo(GStatus.selectedPaths)
        canFullScreen = (GStatus.selectedPaths.length === 1 && FileControl.pathExists(GStatus.selectedPaths[0]))
                && FileControl.isImage(GStatus.selectedPaths[0]) && !isInTrash
        canSlideShow = ((GStatus.selectedPaths.length === 1 && FileControl.pathExists(GStatus.selectedPaths[0])) || haveImage)
                && FileControl.isImage(GStatus.selectedPaths[0]) && !isInTrash
        canView = GStatus.selectedPaths.length === 1 && !isInTrash
        canExport= GStatus.selectedPaths.length >= 1 && FileControl.pathExists(GStatus.selectedPaths[0]) && haveImage && !haveVideo && !isInTrash
        canCopy = GStatus.selectedPaths.length >= 1 && !isInTrash
        canDelete = FileControl.isCanDelete(GStatus.selectedPaths)
        canViewPhotoInfo = GStatus.selectedPaths.length === 1 && FileControl.isImage(GStatus.selectedPaths[0])
        canViewVideoInfo = GStatus.selectedPaths.length === 1 && !FileControl.isImage(GStatus.selectedPaths[0])
        canWallpaper = GStatus.selectedPaths.length === 1 && FileControl.isSupportSetWallpaper(GStatus.selectedPaths[0]) && !isInTrash
        canRotate = FileControl.isRotatable(GStatus.selectedPaths) && !isInTrash
        canDisplayInFolder = GStatus.selectedPaths.length === 1 && FileControl.pathExists(GStatus.selectedPaths[0]) && !isInTrash
        canPrint = FileControl.isCanPrint(GStatus.selectedPaths) && GStatus.selectedPaths.length >= 1 && !isInTrash
    }

    // 执行图片查看操作
    function executeViewImageCutSwitch(url, allUrls) {
        if (url !== undefined) {
            if (FileControl.isVideo(url)) {
                albumControl.openLingmoMovie(url)
            } else {
                GControl.setImageFiles(allUrls, url)
                FileControl.resetImageFiles(allUrls);
                mainStack.switchImageView()
                GStatus.stackControlCurrent = 1
            }
        }
    }

    // 执行图片删除操作
    function executeDelete() {
        albumControl.insertTrash(GStatus.selectedPaths)
    }

    // 执行全屏预览
    function executeFullScreen(url, allUrls) {
        if (window.visibility !== Window.FullScreen && selectedUrls.length > 0) {
            GControl.setImageFiles(allUrls, url)
            FileControl.resetImageFiles(allUrls);
            mainStack.switchImageView()
            GStatus.stackControlLastCurrent = GStatus.stackControlCurrent
            GStatus.stackControlCurrent = 1
            GStatus.showFullScreen = true
        }
    }

    // 执行图片打印
    function executePrint() {
        FileControl.showPrintDialog(GStatus.selectedPaths)
    }

    // 执行幻灯片放映
    function excuteSlideShow(allUrls) {
        if (canSlideShow) {
            stackControl.startMainSliderShow(allUrls, allUrls.indexOf(GStatus.selectedPaths[0]))
        }
    }

    // 执行导出图片
    function excuteExport() {
        if (GStatus.selectedPaths.length > 1) {
            var bRet = albumControl.getFolders(GStatus.selectedPaths)
            if (bRet)
                DTK.sendMessage(thumbnailImage, qsTr("Export successful"), "notify_checked")
            else
                DTK.sendMessage(thumbnailImage, qsTr("Export failed"), "warning")
        } else {
            exportdig.setParameter(GStatus.selectedPaths[0], window)
            exportdig.show()
        }
    }

    // 执行图片复制
    function executeCopy() {
        if (GStatus.selectedPaths.length > 0)
            FileControl.copyImage(GStatus.selectedPaths)
    }

    // 执行从相册移除
    function executeRemoveFromAlbum() {
        if (selectedUrls.length > 0) {
            albumControl.removeFromAlbum(GStatus.currentCustomAlbumUId, selectedUrls)
            GStatus.sigFlushCustomAlbumView(GStatus.currentCustomAlbumUId)
        }
    }

    // 执行收藏操作
    function executeFavorite() {
        albumControl.insertIntoAlbum(0, GStatus.selectedPaths)
        GStatus.bRefreshFavoriteIconFlag = !GStatus.bRefreshFavoriteIconFlag

        // 若当前视图为我的收藏，需要实时刷新我的收藏列表内容
        if (GStatus.currentViewType === Album.Types.ViewFavorite && GStatus.currentCustomAlbumUId === 0) {
            GStatus.sigFlushCustomAlbumView(GStatus.currentCustomAlbumUId)
        }
    }

    // 执行取消收藏操作
    function executeUnFavorite() {
        albumControl.removeFromAlbum(0, GStatus.selectedPaths)
        GStatus.bRefreshFavoriteIconFlag = !GStatus.bRefreshFavoriteIconFlag

        // 若当前视图为我的收藏，需要实时刷新我的收藏列表内容
        if (GStatus.currentViewType === Album.Types.ViewFavorite && GStatus.currentCustomAlbumUId === 0) {
            GStatus.sigFlushCustomAlbumView(GStatus.currentCustomAlbumUId)
        }
    }

    // 执行旋转操作
    function executeRotate(angle) {
        if (GStatus.selectedPaths.length > 0) {
            FileControl.rotateFile(GStatus.selectedPaths, angle)
        }
    }

    // 执行设置壁纸操作
    function executeSetWallpaper() {
        if (GStatus.selectedPaths.length > 0)
            FileControl.setWallpaper(GStatus.selectedPaths[0])
    }

    // 执行在文管中显示操作
    function executeDisplayInFileManager() {
        if (GStatus.selectedPaths.length > 0)
            FileControl.displayinFileManager(GStatus.selectedPaths[0])
    }

    // 执行图片恢复操作
    function executeRestore() {
        if (selectedPaths.length > 0) {
            albumControl.recoveryImgFromTrash(selectedPaths)
            GStatus.sigFlushRecentDelView()
        }
    }

    // 执行照片信息查看
    function executeViewPhotoInfo() {
        albumInfomationDig.filePath = GStatus.selectedPaths[0]
        albumInfomationDig.show()
    }

    // 执行视频信息查看
    function executeViewVideoInfo() {
        videoInfomationDig.filePath = GStatus.selectedPaths[0]
        videoInfomationDig.show()
    }
}
