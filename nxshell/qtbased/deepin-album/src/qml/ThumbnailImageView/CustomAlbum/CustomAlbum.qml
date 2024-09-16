// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0

import org.deepin.album 1.0 as Album

import "../../Control"
import "../../Control/ListView"
import "../../"
import "../"

BaseView {

    property int customAlbumUId: GStatus.currentCustomAlbumUId
    property string customAlbumName: "" //相册名称显示内容
    property int totalPictrueAndVideos: 0
    property int filterType: filterCombo.currentIndex // 筛选类型，默认所有
    property string numLabelText: "" //总数标签显示内容
    property string selectedText: getSelectedText(selectedPaths)
    property alias selectedPaths: theView.selectedUrls
    property bool isCustom : albumControl.isCustomAlbum(customAlbumUId)
    property bool isSystemAutoImport: albumControl.isSystemAutoImportAlbum(customAlbumUId)
    property bool isNormalAutoImport: albumControl.isNormalAutoImportAlbum(customAlbumUId)
    property bool allowFlushcontent: {
        if (GStatus.currentViewType === Album.Types.ViewFavorite && GStatus.currentCustomAlbumUId === 0) {
            return true
        } else if (GStatus.currentViewType === Album.Types.ViewCustomAlbum && GStatus.currentCustomAlbumUId > 0) {
            return true
        } else
            return false
    }

    onVisibleChanged: {
        if (visible && !GStatus.backingToMainAlbumView) {
            flushAlbumName(GStatus.currentCustomAlbumUId, albumControl.getCustomAlbumByUid(GStatus.currentCustomAlbumUId))
            flushCustomAlbumView(GStatus.currentCustomAlbumUId)
        }
    }

    // 筛选类型改变处理事件
    onFilterTypeChanged: {
        if (visible)
            flushCustomAlbumView(GStatus.currentCustomAlbumUId)
    }

    // 我的收藏和相册视图之间切换，需要重载数据
    onCustomAlbumUIdChanged: {
        if (visible) {
            if (GStatus.currentViewType === Album.Types.ViewCustomAlbum && GStatus.currentCustomAlbumUId > 0)
                showAnimation.start()
            flushAlbumName(GStatus.currentCustomAlbumUId, albumControl.getCustomAlbumByUid(GStatus.currentCustomAlbumUId))
            flushCustomAlbumView(GStatus.currentCustomAlbumUId)
        }
    }

    // 刷新自定义相册名称
    function flushAlbumName(UID, name) {
        if (UID === GStatus.currentCustomAlbumUId && allowFlushcontent) {
            customAlbumName = name
        }
    }

    // 刷新自定义相册/我的收藏视图内容
    function flushCustomAlbumView(UID) {
        if (UID === GStatus.currentCustomAlbumUId || UID === -1) {
            if (!allowFlushcontent)
                return
            dataModel.albumId = customAlbumUId
            theView.proxyModel.refresh(filterType)
            GStatus.selectedPaths = theView.selectedUrls
            getNumLabelText()
        }
    }

    // 刷新总数标签
    function getNumLabelText() {
        //QML的翻译不支持%n的特性，只能拆成这种代码

        var photoCountText = ""
        var photoCount = albumControl.getCustomAlbumInfoConut(customAlbumUId, 1)
        if(photoCount === 0) {
            photoCountText = ""
        } else if(photoCount === 1) {
            photoCountText = qsTr("1 photo")
        } else {
            photoCountText = qsTr("%1 photos").arg(photoCount)
        }

        var videoCountText = ""
        var videoCount = albumControl.getCustomAlbumInfoConut(customAlbumUId, 2)
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

    // 刷新选中项目标签内容
    function getSelectedText(paths) {
        var selectedNumText = GStatus.getSelectedNumText(paths, numLabelText)
        if (visible)
            GStatus.statusBarNumText = selectedNumText
        return selectedNumText
    }

    Connections {
        target: albumControl
        function onSigRepeatUrls(urls) {
            if (visible && GStatus.currentCustomAlbumUId !== 0) {
                theView.selectUrls(urls)
            }
        }
    }

    // 自定义相册标题栏区域
    Item {
        id: customAlbumTitleRect
        width: parent.width - GStatus.verticalScrollBarWidth
        height: GStatus.thumbnailViewTitleHieght - 10
        // 相册名称标签
        Label {
            id: customAlbumLabel
            anchors {
                top: parent.top
                topMargin: 12
                left: parent.left
            }
            height: 30
            font: DTK.fontManager.t3
            text: qsTr(customAlbumName)
        }

        Label {
            id: customAlbumNumLabel
            anchors {
                top: customAlbumLabel.bottom
                topMargin: 10
                left: parent.left
            }
            font: DTK.fontManager.t6
            text: numLabelText !== "" ? numLabelText : qsTr("0 item")
        }

        // 筛选下拉框
        FilterComboBox {
            id: filterCombo
            anchors {
                top: customAlbumLabel.bottom
                topMargin: 4
                right: parent.right
            }
            width: 115
            height: 30
        }

        MouseArea {
            anchors.fill: parent
            onPressed: (mouse)=> {
                theView.selectAll(false)
                mouse.accepted = false
            }
        }
    }

    // 缩略图列表控件
    ThumbnailListViewAlbum {
        id: theView
        anchors {
            top: customAlbumTitleRect.bottom
            topMargin: 10
        }
        width: parent.width
        height: parent.height - customAlbumTitleRect.height - m_topMargin
        thumnailListType: (GStatus.currentViewType === Album.Types.ViewCustomAlbum && GStatus.currentCustomAlbumUId > 3) ? Album.Types.ThumbnailCustomAlbum
                                                                                                                                    : Album.Types.ThumbnailNormal
        proxyModel.sourceModel: Album.ImageDataModel { id: dataModel; modelType: Album.Types.CustomAlbum}

        visible: numLabelText !== ""
        property int m_topMargin: 10

        // 监听缩略图列表选中状态，一旦改变，更新globalVar所有选中路径
        Connections {
            target: theView
            function onSelectedChanged() {
                if (parent.visible)
                    GStatus.selectedPaths = theView.selectedUrls
            }
        }
    }

    // 筛选无内容时，显示无结果
    Label {
        anchors {
            top: customAlbumTitleRect.bottom
            left: parent.left
            bottom: theView.bottom
            right: parent.right
            centerIn: parent
        }
        visible: numLabelText === "" && filterType > 0
        font: DTK.fontManager.t4
        color: Qt.rgba(85/255, 85/255, 85/255, 0.4)
        text: qsTr("No results")
    }

    // 1.自定义相册，若没有数据，显示导入图片视图
    // 2.自动导入相册，无内容时，显示没有图片或视频时显示
    NoPictureView {
        visible: numLabelText === ""  && filterType === 0
        bShowImportBtn: isCustom
        iconName: isCustom ? "nopicture1" : (GStatus.currentViewType === Album.Types.ViewCustomAlbum ? "nopicture2" : "nopicture3")
    }

    NumberAnimation {
        id: showAnimation
        target: theView
        property: "anchors.topMargin"
        from: 10 + theView.height
        to: 10
        duration: GStatus.animationDuration
        easing.type: Easing.OutExpo
    }

    onShowChanged: {
        if (show)
            showAnimation.start()
    }

    Component.onCompleted: {
        GStatus.sigFlushCustomAlbumView.connect(flushCustomAlbumView)
        GStatus.sigCustomAlbumNameChaged.connect(flushAlbumName)
    }
}
