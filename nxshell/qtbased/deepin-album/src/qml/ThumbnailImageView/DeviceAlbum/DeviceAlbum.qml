// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0

import org.deepin.album 1.0 as Album

import "../../Control"
import "../../Control/ListView"
import "../../"

BaseView {
    anchors.fill: parent

    property int customAlbumUId: 0
    property string devicePath: GStatus.currentDevicePath
    property string deviceName: GStatus.currentDeviceName
    property int filterType: 0
    property int currentImportIndex: 0
    property var numLabelText: getNumLabelText(filterType)
    property string selectedText: getSelectedText(selectedPaths)
    property alias selectedPaths: theView.selectedPaths

    onVisibleChanged: {
        if (!GStatus.backingToMainAlbumView)
            flushDeviceAlbumView()
    }

    // 筛选类型改变处理事件
    onFilterTypeChanged: {
        flushDeviceAlbumView()
    }

    // 设备之间切换，需要重载数据
    onDevicePathChanged: {
        flushDeviceAlbumView()
    }

    // 刷新设备视图内容
    function flushDeviceAlbumView() {
        if (!visible)
            return

        dataModel.devicePath = devicePath
        theView.proxyModel.refresh(filterType)
        GStatus.selectedPaths = theView.selectedUrls
        getNumLabelText()
    }

    // 刷新总数标签
    function getNumLabelText() {
        //QML的翻译不支持%n的特性，只能拆成这种代码

        var photoCountText = ""
        var photoCount = albumControl.getDeviceAlbumInfoConut(devicePath, 3)
        if(photoCount === 0) {
            photoCountText = ""
        } else if(photoCount === 1) {
            photoCountText = qsTr("1 photo")
        } else {
            photoCountText = qsTr("%1 photos").arg(photoCount)
        }

        var videoCountText = ""
        var videoCount = albumControl.getDeviceAlbumInfoConut(devicePath, 4)
        if(videoCount === 0) {
            videoCountText = ""
        } else if(videoCount === 1) {
            videoCountText = qsTr("1 video")
        } else {
            videoCountText = qsTr("%1 videos").arg(videoCount)
        }

        var numLabelText = filterType == 0 ? (photoCountText + (videoCountText !== "" ? ((photoCountText !== "" ? " " : "") + videoCountText) : ""))
                                           : (filterType == 1 ? photoCountText : videoCountText)
        if (visible) {
            GStatus.statusBarNumText = numLabelText
        }

        return numLabelText
    }

    // 刷新选中项数标签
    function getSelectedText(paths) {
        var selectedNumText = GStatus.getSelectedNumText(paths, numLabelText)
        if (visible)
            GStatus.statusBarNumText = selectedNumText
        return selectedNumText
    }

    DeviceLoadDialog {
        id: devloaddig
    }

    // 新增挂载设备，显示正在加载对话框
    Connections {
        target: albumControl
        function onSigAddDevice() {
            devloaddig.show()
        }
    }

    // 导入重复图片提示
    Connections {
        target: albumControl
        function onSigRepeatUrls(urls) {
            if (visible && GStatus.currentCustomAlbumUId !== 0) {
                theView.selectedPaths = urls
                GStatus.selectedPaths = selectedPaths
            }
        }
    }

    // 设备相册标题栏区域
    Item {
        id: deviceAlbumTitleRect
        width: parent.width - GStatus.verticalScrollBarWidth
        height: GStatus.thumbnailViewTitleHieght - 10
        // 设备名称标签
        Label {
            id: deviceAlbumLabel
            anchors {
                top: parent.top
                topMargin: 12
                left: parent.left
            }
            height: 30
            font: DTK.fontManager.t3
            text: qsTr(deviceName)
        }

        Label {
            id: deviceAlbumNumLabel
            anchors {
                top: deviceAlbumLabel.bottom
                topMargin: 10
                left: parent.left
            }
            font: DTK.fontManager.t6
            text: numLabelText
        }
        Row{
            anchors {
                bottom: parent.bottom
                right: parent.right
            }
            height: 36
            spacing: 10
            // 筛选下拉框
            Item{
                width: 80
                height: 36
                Label {
                    anchors.centerIn: parent
                    text :qsTr("Import to:")
                    width: 56
                    height: 20
                }
            }

            ComboBox {
                id: filterCombo
                width: 180
                height: 36
                displayText : currentIndex == 0?qsTr("Import") : currentIndex == 1? qsTr("New Album")  :albumControl.getAllCustomAlbumName(GStatus.albumChangeList)[currentIndex-2]
                model: albumControl.getAllCustomAlbumId(GStatus.albumChangeList).length+2
                delegate: MenuItem {
                    text: index == 0?qsTr("Import") : index == 1? qsTr("New Album")  :albumControl.getAllCustomAlbumName(GStatus.albumChangeList)[index-2]
                    onTriggered:{
                        if(index == 0 ){
                            currentImportIndex = 0
                        }else if(index == 1){
                            var x = parent.mapToGlobal(0, 0).x + parent.width / 2 - 190
                            var y = parent.mapToGlobal(0, 0).y + parent.height / 2 - 89
                            newAlbum.setX(x)
                            newAlbum.setY(y)
                            newAlbum.isChangeView = false
                            newAlbum.setNormalEdit()
                            newAlbum.show()
                        }else {
                            currentImportIndex = index -2
                        }
                    }
                    highlighted: filterCombo.highlightedIndex === index

                }

            }
            RecommandButton{
                id: openPictureBtn
                font.capitalization: Font.MixedCase
                text: GStatus.selectedPaths.length === 0 ? qsTr("Import All")
                                                       : GStatus.selectedPaths.length === 1 ? qsTr("Import 1 Item")
                                                       : qsTr("Import %1 Items").arg(GStatus.selectedPaths.length)
                onClicked:{
                    if(GStatus.selectedPaths.length > 0){
                        albumControl.importFromMountDevice(GStatus.selectedPaths,albumControl.getAllCustomAlbumId(GStatus.albumChangeList)[currentImportIndex])
                    }else{
                        albumControl.importFromMountDevice(theView.allUrls(),albumControl.getAllCustomAlbumId(GStatus.albumChangeList)[currentImportIndex])
                    }
                    DTK.sendMessage(thumbnailImage, qsTr("Import successful"), "notify_checked")
                }
                width: 114
                height: 36
            }
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
            top: deviceAlbumTitleRect.bottom
            topMargin: 10
        }
        width: parent.width
        height: parent.height - deviceAlbumTitleRect.height - m_topMargin
        visible: numLabelText !== ""
        thumnailListType: Album.Types.ThumbnailDevice

        proxyModel.sourceModel: Album.ImageDataModel { id: dataModel; modelType: Album.Types.Device}

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

    Label {
        anchors {
            top: deviceAlbumTitleRect.bottom
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
}
