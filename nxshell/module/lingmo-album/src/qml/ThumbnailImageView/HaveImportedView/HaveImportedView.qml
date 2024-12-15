// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.lingmo.dtk 1.0

import org.lingmo.album 1.0 as Album

import "../../Control"
import "../../"
import "../"

BaseView {
    id: haveImportedListView
    property int filterType : timeline.filterType// 筛选类型，默认所有
    property string numLabelText: "" //总数标签显示内容
    property string selectedText: getSelectedText(selectedPaths)
    property var selectedPaths: GStatus.selectedPaths
    property real titleOpacity: 0.7
    property bool bShowImportTips: numLabelText === ""
                                   && filterType === 0
                                   && albumControl.getAllCount() === 0

    Rectangle {
        anchors.fill : parent
        color: DTK.themeType === ApplicationHelper.LightType ? "#f8f8f8"
                                                              : "#202020"
        visible: !bShowImportTips

        Album.QmlWidget{
            id: timeline
            anchors.fill: parent
            anchors.margins: 0
            focus: false
            viewType: Album.Types.WidgetImportedView
        }
    }

    Connections {
        target: GStatus
        function onSigDoubleClickedFromQWidget(url) {
            if (!haveImportedListView.visible)
                return
            if (url !== undefined) {
                var allUrls = timeline.allUrls()
                menuItemStates.executeViewImageCutSwitch(url, allUrls)
            }
        }

        function onSigMenuItemClickedFromQWidget(id, uid) {
            if (!haveImportedListView.visible)
                return
            var sels = GStatus.selectedPaths
            var url = ""
            if (sels.length > 0)
                url = sels[0]
            var allUrls = timeline.allUrls()
            if (id === Album.Types.IdView) {
                menuItemStates.executeViewImageCutSwitch(url, allUrls)
            } else if (id === Album.Types.IdMoveToTrash) {
                deleteDialog.setDisplay(Album.Types.TrashNormal, GStatus.selectedPaths.length)
                deleteDialog.show()
            } else if (id === Album.Types.IdFullScreen) {
                menuItemStates.executeFullScreen(url, allUrls)
            } else if (id === Album.Types.IdPrint) {
                menuItemStates.executePrint()
            } else if (id === Album.Types.IdStartSlideShow) {
                menuItemStates.excuteSlideShow(allUrls)
            } else if (id === Album.Types.IdExport) {
                menuItemStates.excuteExport()
            } else if (id === Album.Types.IdCopyToClipboard) {
                menuItemStates.executeCopy()
            } else if (id === Album.Types.IdAddToFavorites) {
                menuItemStates.executeFavorite()
            } else if (id === Album.Types.IdRemoveFromFavorites) {
                menuItemStates.executeUnFavorite()
            } else if (id === Album.Types.IdRotateClockwise) {
                menuItemStates.executeRotate(90)
            } else if (id === Album.Types.IdRotateCounterclockwise) {
                menuItemStates.executeRotate(-90)
            } else if (id === Album.Types.IdSetAsWallpaper) {
                menuItemStates.executeSetWallpaper()
            } else if (id === Album.Types.IdDisplayInFileManager) {
                menuItemStates.executeDisplayInFileManager()
            } else if (id === Album.Types.IdImageInfo) {
                menuItemStates.executeViewPhotoInfo()
            } else if (id === Album.Types.IdVideoInfo) {
                menuItemStates.executeViewVideoInfo()
            } else if (id === Album.Types.IdNewAlbum) {
                newAlbum.isChangeView = false
                newAlbum.importSelected = true
                newAlbum.setNormalEdit()
                newAlbum.show()
            } else if (id === Album.Types.IdAddToAlbum) {
                // 获取所选自定义相册的Id，根据Id添加到对应自定义相册
                albumControl.insertIntoAlbum(uid , GStatus.selectedPaths)
                DTK.sendMessage(thumbnailImage, qsTr("Successfully aoceand to “%1”").arg(albumControl.getCustomAlbumByUid(uid)), "notify_checked")
            }
        }
    }

    onVisibleChanged: {
        if (visible && !GStatus.backingToMainAlbumView) {
            flushView()
            if (show)
                showAnimation.start()
        }
    }

    // 筛选类型改变处理事件
    onFilterTypeChanged: {
        if (visible)
            getNumLabelText()
    }

    // 刷新已导入视图内容
    function flushView() {
        if (!visible)
            return
        unSelectAll()
        timeline.refresh()
        getNumLabelText()
    }

    function unSelectAll() {
        timeline.unSelectAll()
    }

    function runDeleteImg() {
        menuItemStates.executeDelete()
        getNumLabelText()
    }

    Connections {
        target: albumControl
        function onSigRepeatUrls(urls) {
            if (visible) {
                timeline.selectUrls(urls)
            }
        }
    }

    // 刷新总数标签
    function getNumLabelText() {
        //QML的翻译不支持%n的特性，只能拆成这种代码

        var photoCountText = ""
        var photoCount = albumControl.getAllInfoConut(1)
        if(photoCount === 0) {
            photoCountText = ""
        } else if(photoCount === 1) {
            photoCountText = qsTr("1 photo")
        } else {
            photoCountText = qsTr("%1 photos").arg(photoCount)
        }

        var videoCountText = ""
        var videoCount = albumControl.getAllInfoConut(2)
        if(videoCount === 0) {
            videoCountText = ""
        } else if(videoCount === 1) {
            videoCountText = qsTr("1 video")
        } else {
            videoCountText = qsTr("%1 videos").arg(videoCount)
        }

        numLabelText = filterType == 0 ? (photoCountText + (videoCountText !== "" ? ((photoCountText !== "" ? " " : "") + videoCountText) : ""))
                                           : (filterType == 1 ? photoCountText : videoCountText)
        if (visible) {
            GStatus.statusBarNumText = numLabelText
        }
    }

    // 刷新选中项数标签
    function getSelectedText(paths) {
        var selectedNumText = GStatus.getSelectedNumText(paths, numLabelText)
        if (visible)
            GStatus.statusBarNumText = selectedNumText
        return selectedNumText
    }

    Connections {
        target: GStatus

        function onSigSelectAll(sel) {
            if (visible) {
                if (sel)
                    GStatus.sigKeyPressFromQml("Ctrl+A")
                else
                    GStatus.sigKeyPressFromQml("Esc")
            }
        }

        function onSigPageUp() {
            if (visible) {
                GStatus.sigKeyPressFromQml("Page Up")
            }
        }

        function onSigPageDown() {
            if (visible) {
                GStatus.sigKeyPressFromQml("Page Down")
            }
        }
    }

    // 若没有数据，显示导入图片视图
    NoPictureView {
        visible: bShowImportTips
        bShowImportBtn: true
        iconName: "nopicture1"
    }

    NumberAnimation {
        id: showAnimation
        target: timeline
        property: "anchors.topMargin"
        from: timeline.height
        to: 0
        duration: GStatus.sidebarAnimationEnabled ? GStatus.animationDuration : 0
        easing.type: Easing.OutExpo
    }

    Component.onCompleted: {
        GStatus.sigFlushHaveImportedView.connect(flushView)
    }
}
