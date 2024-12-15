// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import "./PreviewImageViewer"
import "./PreviewImageViewer/InformationDialog"
import "./Control"

import org.lingmo.album 1.0 as Album

Item {
    anchors.fill: parent
    //本文件用于替代stackwidget的作用，通过改变GStatus的0-n来切换窗口
    MainAlbumView{
        id: mainAlbumView
        idName: "mainAlbumView"
        show: GStatus.stackControlCurrent === 0
    }
    MainStack{
        id: mainStack
        idName: "mainStack"
        anchors.fill: parent
        show: GStatus.stackControlCurrent === 1
        iconName: "lingmo-album"
    }
    SliderShow{
        id: mainSliderShow
        idName: "albumview"
        anchors.fill: parent
        visible: GStatus.stackControlCurrent === 2
    }

    //全屏动画
    PropertyAnimation {
        id :showfullAnimation
        target: window
        from: 0
        to: 1
        property: "opacity"
        duration: 200
        easing.type: Easing.InExpo
    }

    GlobalVar{
        id: global
    }

    MenuItemStates {
        id: menuItemStates
    }

    //delete窗口
    DeleteDialog {
        id: deleteDialog
    }

    //export窗口
    ExportDialog {
        id: exportdig
    }

    //info的窗口
    InformationDialog{
        id: albumInfomationDig
        visible: false
    }

    //视频info窗口
    VideoInfoDialog{
        id: videoInfomationDig
    }

    //相册界面启动幻灯片（和看图界面启动有区别）
    //images: 图片路径 startIndex: 启动后一个图的下标索引
    function startMainSliderShow(images, startIndex) {
        showFullScreen()
        GControl.setImageFiles(images, images[startIndex])
        FileControl.resetImageFiles(images);
        mainStack.switchImageView()
        mainSliderShow.autoRun = true
        mainSliderShow.source = "image://ImageLoad/" + GControl.currentSource + "#frame_" + GControl.currentFrameIndex;
        mainSliderShow.restart()
        GStatus.stackControlCurrent = 2
        mainAlbumView.visible = false
        showfullAnimation.start()
    }
}
