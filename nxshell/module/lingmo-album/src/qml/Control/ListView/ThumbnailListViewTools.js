// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 执行图片查看操作
function executeViewImageCutSwitch() {
    if (thumnailListType !== Album.Types.ThumbnailTrash) {
        var indexes = thumbnailModel.selectedIndexes
        if (indexes.length > 0) {
            var url = thumbnailModel.data(indexes[0], "url").toString()
            var allUrls = thumbnailModel.allUrls()
            menuItemStates.executeViewImageCutSwitch(url, allUrls)
        }
    }
}

// 执行图片查看操作
function executeViewImage(x, y, w, h) {
    if (thumnailListType !== Album.Types.ThumbnailTrash) {
        var indexes = thumbnailModel.selectedIndexes
        if (indexes.length > 0) {
            var url = thumbnailModel.data(indexes[0], "url").toString()
            var allUrls = thumbnailModel.allUrls()
            if (FileControl.isVideo(url)) {
                menuItemStates.executeViewImageCutSwitch(url, allUrls)
            } else {
                GStatus.enteringImageViewer = true
                menuItemStates.executeViewImageCutSwitch(url, allUrls)
                GStatus.sigMoveCenter(x, y, w, h)
                GStatus.sigShowToolBar()
            }
        }
    }
}
// 执行图片删除操作
function executeDelete() {
    if ( thumnailListType !== Album.Types.ThumbnailTrash ){
        albumControl.insertTrash(GStatus.selectedPaths)
        thumbnailModel.clearSelection()
    } else {
        albumControl.deleteImgFromTrash(selectedPaths)
        selectAll(false)
        GStatus.sigFlushRecentDelView()
    }
}

// 执行全屏预览
function executeFullScreen() {
    if (window.visibility !== Window.FullScreen && selectedUrls.length > 0) {
        var allUrls = thumbnailModel.allUrls()
        var indexes = thumbnailModel.selectedIndexes
        if (indexes.length > 0) {
            menuItemStates.executeFullScreen(allUrls[indexes[0]], allUrls)
        }
    }
}

// 执行图片打印
function executePrint() {
    menuItemStates.executePrint()
}

// 执行幻灯片放映
function excuteSlideShow() {
    if (selectedUrls.length > 0) {
        var allUrls = thumbnailModel.allUrls()
        menuItemStates.excuteSlideShow(allUrls)
        //stackControl.startMainSliderShow(allUrls, allUrls.indexOf(selectedUrls[0]))
    }
}

// 执行导出图片
function excuteExport() {
    menuItemStates.excuteExport()
}

// 执行图片复制
function executeCopy() {
    menuItemStates.executeCopy()
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
    menuItemStates.executeFavorite()
}

// 执行取消收藏操作
function executeUnFavorite() {
    menuItemStates.executeUnFavorite()
}

// 执行旋转操作
function executeRotate(angle) {
    menuItemStates.executeRotate(angle)
}

// 执行设置壁纸操作
function executeSetWallpaper() {
    menuItemStates.executeSetWallpaper()
}

// 执行在文管中显示操作
function executeDisplayInFileManager() {
    menuItemStates.executeDisplayInFileManager()
}

// 执行图片恢复操作
function executeRestore() {
    if (selectedPaths.length > 0) {
        albumControl.recoveryImgFromTrash(selectedPaths)
        selectAll(false)
        GStatus.sigFlushRecentDelView()
    }
}

// 执行照片信息查看
function executeViewPhotoInfo() {
    menuItemStates.executeViewPhotoInfo()
}


// 执行视频信息查看
function executeViewVideoInfo() {
    menuItemStates.executeViewVideoInfo()
}
