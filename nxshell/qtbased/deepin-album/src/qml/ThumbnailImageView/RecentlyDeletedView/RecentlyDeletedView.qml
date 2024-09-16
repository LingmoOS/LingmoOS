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

    property int filterType : filterCombo.currentIndex // 筛选类型，默认所有
    property string numLabelText: ""
    property string selectedText: getSelectedText(selectedPaths)
    property alias selectedPaths: theView.selectedUrls
    property int totalCount: 0

    onVisibleChanged: {
        flushRecentDelView()
    }

    // 筛选类型改变处理事件
    onFilterTypeChanged: {
        flushRecentDelView()
    }

    // 刷新最近删除视图内容
    function flushRecentDelView() {
        if (!visible)
            return

        theView.proxyModel.refresh(filterType)
        GStatus.selectedPaths = theView.selectedUrls
        getNumLabelText()
        totalCount = albumControl.getTrashInfoConut(0)
    }

    // 刷新总数标签
    function getNumLabelText() {
        //QML的翻译不支持%n的特性，只能拆成这种代码

        var photoCountText = ""
        var photoCount = albumControl.getTrashInfoConut(1)
        if(photoCount === 0) {
            photoCountText = ""
        } else if(photoCount === 1) {
            photoCountText = qsTr("1 photo")
        } else {
            photoCountText = qsTr("%1 photos").arg(photoCount)
        }

        var videoCountText = ""
        var videoCount = albumControl.getTrashInfoConut(2)
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

    // 加载最近删除图片数据
    function loadRecentDelItems()
    {
        console.info("recently delete model has refreshed... filterType:", filterType)
        theView.thumbnailListModel.clear();
        var delInfos = albumControl.getTrashAlbumInfos(filterType);
        console.info("recently delete model has refreshed... filterType:", filterType, " done...")
        for (var key in delInfos) {
            var delItems = delInfos[key]
            for (var i = 0; i < delItems.length; i++) {
                theView.thumbnailListModel.append(delItems[i])
            }
            break;
        }
    }

    //执行图片删除操作
    function runAllDeleteImg() {
        albumControl.deleteImgFromTrash(theView.allPaths())
        theView.selectAll(false)
        GStatus.sigFlushRecentDelView()
    }

    // 最近删除标题栏区域
    Item {
        id: recentDelTitleRect
        width: parent.width - GStatus.verticalScrollBarWidth
        height: GStatus.thumbnailViewTitleHieght - 10
        // 最近删除标签
        Label {
            id: recentDelLabel
            anchors {
                top: parent.top
                topMargin: 12
                left: parent.left
            }
            height: 30
            font: DTK.fontManager.t3
            text: qsTr("Trash")
        }

        Label {
            id: recentDelTipLabel
            anchors {
                top: recentDelLabel.bottom
                topMargin: 10
                left: parent.left
            }

            width: restoreSelectedBtn.visible ? (restoreSelectedBtn.x - recentDelLabel.x) : (filterCombo.x - recentDelLabel.x)
            font: DTK.fontManager.t6
            text: textMetics.elidedText

            TextMetrics {
                id: textMetics

                elide: Text.ElideRight
                elideWidth: recentDelTipLabel.width
                font: recentDelTipLabel.font
                text: qsTr("The files will be permanently deleted after the days shown on them")
            }
        }

        //删除全部按钮
        Label {
            id: delAllBtn
            anchors {
                top: parent.top
                topMargin: 12
                right: parent.right
            }
            width: 56
            height: 36
            font: DTK.fontManager.t6
            text: qsTr("Delete All")
            visible: !theView.haveSelect && totalCount
            color: mouseAreaDelAllBtn.containsMouse ? "#FF5736" : "#FD6E52"


            MouseArea {
                id: mouseAreaDelAllBtn
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    deleteDialog.setDisplay(Album.Types.TrashAll, theView.allPaths().length)
                    deleteDialog.show()
                }
            }
        }

        // 筛选下拉框
        FilterComboBox {
            id: filterCombo
            anchors {
                top: recentDelLabel.bottom
                topMargin: 4
                right: parent.right
            }
            width: 115
            height: 30
            visible: (!theView.haveSelect && totalCount > 0) || totalCount === 0
        }

        WarningButton {
            id: delSelectedBtn
            anchors {
                top: recentDelLabel.bottom
                topMargin: 4
                right: parent.right
            }
            width: 140
            height: 36
            font: DTK.fontManager.t6
            text: qsTr("Delete Selected (%1)").arg(theView.haveSelectCount)
            visible: theView.haveSelect

            onClicked: {
                deleteDialog.setDisplay(Album.Types.TrashSel, theView.selectedPaths.length)
                deleteDialog.show()
            }
        }

        Button {
            id: restoreSelectedBtn
            anchors {
                top: recentDelLabel.bottom
                topMargin: 4
                right: delSelectedBtn.left
                rightMargin: 10
            }
            width: 140
            height: 36
            font: DTK.fontManager.t6
            text: qsTr("Restore Selected (%1)").arg(theView.haveSelectCount)
            visible: theView.haveSelect

            onClicked: {
                albumControl.recoveryImgFromTrash(theView.selectedPaths)
                theView.selectAll(false)
                GStatus.sigFlushRecentDelView()
            }
        }

        MouseArea {
            id: theMouseArea
            anchors.fill: parent
            onPressed: (mouse)=> {
                var gPos = theMouseArea.mapToGlobal(mouse.x, mouse.y)
                if (!restoreSelectedBtn.contains(restoreSelectedBtn.mapFromGlobal(gPos.x, gPos.y))
                        && !delSelectedBtn.contains(delSelectedBtn.mapFromGlobal(gPos.x, gPos.y))) {
                    theView.selectAll(false)
                }
                mouse.accepted = false
            }
        }
    }

    // 缩略图列表控件
    ThumbnailListViewAlbum {
        id: theView
        anchors {
            top: recentDelTitleRect.bottom
            topMargin: 10
        }
        width: parent.width
        height: parent.height - recentDelTitleRect.height - m_topMargin
        thumnailListType: Album.Types.ThumbnailTrash

        proxyModel.sourceModel: Album.ImageDataModel { id: dataModel; modelType: Album.Types.RecentlyDeleted}

        visible: theView.count
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

    // 若没有数据，显示无图片视图
    NoPictureView {
        visible: GStatus.currentViewType === Album.Types.ViewRecentlyDeleted && numLabelText === ""/* && filterType === 0*/
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
        GStatus.sigFlushRecentDelView.connect(flushRecentDelView)
        deleteDialog.sigDoAllDeleteImg.connect(runAllDeleteImg)
    }
}
