// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import QtQml
import QtQuick.Shapes 1.10
import org.deepin.dtk 1.0
import Qt5Compat.GraphicalEffects

import org.deepin.album 1.0 as Album

import "../"
import "../../"
import "./"

Item {
    id: main
    //注意：在model里面加进去的变量，这边可以直接进行使用，只是部分位置不好拿到，需要使用变量
    property string m_index
    property string m_displayFlushHelper
    property QtObject modelData

    property int index: model.index
    property bool blank: model.blank
    property bool bShowDamageIcon: image.null
    property bool bSelected: model.selected !== undefined ? model.selected : false
    property bool bHovered: false //属性：是否hover
    property bool bFavorited: {
        GStatus.bRefreshFavoriteIconFlag
        return albumControl.photoHaveFavorited(model.url)
    }
    property bool bShowRemainDays: GStatus.currentViewType === Album.Types.ViewRecentlyDeleted && !model.blank
    property bool bShowVideoLabel: FileControl.isVideo(model.url) && !model.blank
    property Item selectIcon: null
    property Item selectFrame: null
    property Item favoriteBtn: null
    property Item remainDaysLbl: null
    property Item videoLabel: null
    property int nDuration: GStatus.animationDuration

    // 缩略图本体
    Album.QImageItem {
        id: image
        anchors.centerIn: parent
        width: parent.width - 14
        height: parent.height -14
        smooth: true
        image: {
            gridView.bRefresh
            modelData.thumbnail
        }
        fillMode: Album.QImageItem.PreserveAspectFit
        visible: false
        layer.enabled: true
    }

    // 图片保存完成，缩略图区域重新加载当前图片
    Connections {
        target: FileControl
        function onCallSavePicDone(path) {
            if (path === model.url) {
                model.reloadThumbnail
            }
        }
    }

    //圆角遮罩Rectangle
    Rectangle {
        id: maskRec
        anchors.centerIn: parent
        width: image.width
        height: image.height

        color:"transparent"
        Rectangle {
            anchors.centerIn: parent
            width: image.paintedWidth
            height: image.paintedHeight
            color:"black"
            radius: 10

            Behavior on width {
                enabled: GStatus.enableRatioAnimation
                NumberAnimation {
                    duration: nDuration
                    easing.type: Easing.OutExpo // 缓动类型
                }
            }

            Behavior on height {
                enabled: GStatus.enableRatioAnimation
                NumberAnimation {
                    duration: nDuration
                    easing.type: Easing.OutExpo // 缓动类型
                }
            }
        }

        visible: false
    }

    OpacityMask{
        id: opacityMask
        anchors.fill: maskRec
        source: image
        maskSource: mask
    }

    FastBlur {
        anchors.top: opacityMask.top; anchors.topMargin: 6
        anchors.left: opacityMask.left; anchors.leftMargin: 1
        width: opacityMask.width - 2; height: opacityMask.height - 6
        source: opacityMask
        radius: 10
        transparentBorder: true
    }

    //遮罩执行
    OpacityMask {
        id: mask
        anchors.fill: maskRec
        source: image
        maskSource: maskRec
        antialiasing: true
        smooth: true
    }

    //border and shadow
    Rectangle {
        id: borderRect
        anchors.centerIn: parent
        width: image.paintedWidth
        height: image.paintedHeight
        color: Qt.rgba(0, 0, 0, 0)
        border.color: Qt.rgba(0, 0, 0, 0.1)
        border.width: 1
        visible: true
        radius: 10

        Behavior on width {
            enabled: GStatus.enableRatioAnimation
            NumberAnimation {
                duration: nDuration
                easing.type: Easing.OutExpo // 缓动类型
            }
        }

        Behavior on height {
            enabled: GStatus.enableRatioAnimation
            NumberAnimation {
                duration: nDuration
                easing.type: Easing.OutExpo // 缓动类型
            }
        }
    }

    MouseArea {
        id:mouseAreaTopParentRect
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true

        onClicked: (mouse)=> {
            //允许鼠标事件传递给子控件处理,否则鼠标点击缩略图收藏图标不能正常工作
            mouse.accepted = false
        }

        onEntered: {
            if (model.blank)
                return;

            bHovered = true
            if (favoriteBtn == null && model.modelType !== Album.Types.Device && model.modelType !== Album.Types.RecentlyDeleted)
                favoriteBtn = favoriteComponent.createObject(main)
        }

        onExited: {
            bHovered = false
            if (favoriteBtn && !bFavorited) {
                favoriteBtn.destroy()
                favoriteBtn = null
            }
        }
    }

    onBSelectedChanged: {
        if (bSelected) {
            if (selectIcon == null)
                selectIcon = selectedIconComponent.createObject(main)
            if (selectFrame == null)
                selectFrame = selectedFrameComponent.createObject(main)
        } else {
            selectIcon.destroy()
            selectIcon = null
            selectFrame.destroy()
            selectFrame = null
        }
    }

    onBFavoritedChanged: {
        if (bFavorited) {
            if (favoriteBtn == null) {
                favoriteBtn = favoriteComponent.createObject(main)
            }
        } else {
            if (favoriteBtn && !bHovered) {
                favoriteBtn.destroy()
                favoriteBtn = null
            }
        }
    }

    onBShowRemainDaysChanged: {
        if (bShowRemainDays) {
            if (remainDaysLbl == null) {
                remainDaysLbl = remainDaysComponent.createObject(main)
            }
        } else {
            remainDaysLbl.destroy()
            remainDaysLbl = null
        }
    }

    onBShowVideoLabelChanged: {
        if (bShowVideoLabel) {
            if (videoLabel == null) {
                videoLabel = videoTimeComponent.createObject(main)
            }
        } else {
            videoLabel.destroy()
            videoLabel = null
        }
    }

    // 选中图标组件
    Component {
        id: selectedIconComponent
        Item {
            id: iconArea

            anchors.centerIn: parent// 确保阴影框居中于图片
            width: borderRect.width + 14
            height: borderRect.height + 14

            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 2
                anchors.rightMargin: 2
                color: Qt.rgba(0,0,0,0)
                width: 20
                height: 20


                // 选中后显示的图标
                DciIcon {
                    id: selectIcon
                    name: "select_active_1"
                    visible: true
                    anchors.centerIn: parent
                }

                // 内部阴影
                DciIcon {
                    id: innerShadow
                    name: "Inner_shadow"
                    visible: true
                    anchors.centerIn: parent
                }

                // 外部阴影
                DciIcon {
                    id: outerShadow
                    name: "shadow"
                    visible: true
                    anchors.centerIn: parent
                }

                // 确认图标
                DciIcon {
                    id: confirmIcon
                    name: "yes"
                    visible: true
                    anchors.centerIn: parent
                }
            }
        }
    }

    // 选中框组件
    Component {
        id: selectedFrameComponent
        Item {
            anchors.fill: borderRect

            z: -1

            // 计算图片区域的位置
            Rectangle {
                id: imageArea
                anchors.fill: parent
                visible: false
            }

            // 选中后显示的阴影框
            Rectangle {
                id: selectShader
                anchors.centerIn: parent// 确保阴影框居中于图片
                width: imageArea.width + 14
                height: imageArea.height + 14
                radius: 10
                color: "#AAAAAA"
                visible: true
                opacity: 0.4

                border.color: Qt.rgba(0,0,0,0.1)
                border.width: 1
            }

            //遮罩执行
            OpacityMask {
                id: mask
                anchors.fill: imageArea
                source: imageArea
                maskSource: selectShader
                antialiasing: true
                smooth: true
            }
        }
    }

    // 收藏图标组件
    Component {
        id: favoriteComponent
        Item {
            anchors.fill: borderRect

            //收藏图标
            ActionButton {
                id: itemFavoriteBtn
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    leftMargin : 5
                    bottomMargin : 5
                }

                hoverEnabled: false  //设置为false，可以解决鼠标移动到图标附近时，图标闪烁问题

                icon {
                    name: bFavorited ? "collected" : "collection2"
                }

                MouseArea {
                    id:mouseAreaFavoriteBtn
                    anchors.fill: itemFavoriteBtn
                    propagateComposedEvents: true

                    onClicked: (mouse)=> {
                        var paths = []
                        paths.push(modelData.url)

                        if (bFavorited) {
                            //取消收藏
                            albumControl.removeFromAlbum(0, paths)
                        } else {
                            //收藏
                            albumControl.insertIntoAlbum(0, paths)
                        }

                        GStatus.bRefreshFavoriteIconFlag = !GStatus.bRefreshFavoriteIconFlag
                        // 若当前视图为我的收藏，需要实时刷新我的收藏列表内容
                        if (GStatus.currentViewType === Album.Types.ViewFavorite && GStatus.currentCustomAlbumUId === 0) {
                            GStatus.sigFlushCustomAlbumView(GStatus.currentCustomAlbumUId)
                        }

                        mouse.accepted = true
                    }
                }
            }
        }
    }

    //剩余天数标签组件
    Component {
        id: remainDaysComponent
        Item {
            anchors.fill: borderRect

            VideoLabel {
                id: labelRemainDays
                visible: true
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    leftMargin : 9
                    bottomMargin : 5
                }
                opacity: 0.7
                displayStr: model.remainDays > 1 ? (model.remainDays + qsTr("days")) : (model.remainDays + qsTr("day"))
            }
        }
    }

    // 视频时长组件
    Component {
        id: videoTimeComponent
        Item {
            anchors.fill: borderRect

            VideoLabel {
                id: videoLabel
                visible: bShowVideoLabel
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    rightMargin : 9
                    bottomMargin : 5
                }
                opacity: 0.7
                displayStr: FileControl.isVideo(model.url) ? albumControl.getVideoTime(model.url) : "00:00"

                Connections {
                    target: albumControl
                    function onSigRefreashVideoTime(url, videoTimeStr) {
                        if (url === model.url) {
                            videoLabel.displayStr = videoTimeStr
                        }
                    }
                }
            }
        }
    }
}
