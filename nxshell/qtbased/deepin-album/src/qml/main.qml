// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import org.deepin.dtk 1.0
import org.deepin.album 1.0 as Album

import "./Control/Animation"

ApplicationWindow {
    id: window

    property bool isFullScreen: window.visibility === Window.FullScreen

    // Bug fix: 使用 ListView 替换 PathView 时，出现内部的 mouseArea 鼠标操作会被 DWindow 截取
    // 导致 flicking 时拖动窗口，此处使用此标志禁用此行为
    DWindow.enableSystemMove: !GStatus.viewFlicking

    // 设置 dtk 风格窗口
    DWindow.enabled: true
    DWindow.alphaBufferSize: 8
    title: ""
    header: AlbumTitle {id: titleAlubmRect}

    background: Rectangle {
        anchors.fill: parent
        visible: GStatus.stackControlCurrent === 0 ? true : false
        color: "transparent"
        Row {
            anchors.fill: parent
            BehindWindowBlur {
                id: leftBgArea
                width: GStatus.sideBarWidth
                height: parent.height
                anchors.top: parent.top
                blendColor: DTK.themeType === ApplicationHelper.LightType ? "#eaf7f7f7"
                                                                          : "#ee252525"
                Rectangle {
                    width: 1
                    height: parent.height
                    anchors.right: parent.right
                    color: DTK.themeType === ApplicationHelper.LightType ? "#eee7e7e7"
                                                                         : "#11a2a2a2"
                }
            }
            Rectangle {
                id: rightBgArea
                width: parent.width
                height: 50
                anchors.top: parent.top
                color: Qt.rgba(0, 0, 0, 0.01)
                BoxShadow {
                    anchors.fill: rightBgArea
                    shadowOffsetX: 0
                    shadowOffsetY: 4
                    shadowColor: Qt.rgba(0, 0, 0, 0.05)
                    shadowBlur: 10
                    cornerRadius: rightBgArea.radius
                    spread: 0
                    hollow: true
                }
            }
        }
    }

    MessageManager.layout: Column {
        anchors {
            bottom: parent.bottom
            bottomMargin: GStatus.statusBarHeight + 5
            horizontalCenter: parent.horizontalCenter
        }
    }

    visible: true
    minimumHeight: GStatus.minHeight
    minimumWidth: GStatus.minWidth
    width: FileControl.getlastWidth()
    height: FileControl.getlastHeight()

    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    Component.onCompleted: {
        setX(screen.width / 2 - width / 2);
        setY(screen.height / 2 - height / 2);

        // 合集-所有项视图延迟刷新，解决其加载时会闪烁显示一张缩略图的问题
        GStatus.currentViewType = Album.Types.ViewCollecttion
        GStatus.currentDeviceName = albumControl.getDeviceName(GStatus.currentDevicePath)
    }

    onActiveChanged: {
        // 记录应用主窗口是否被置灰过
        if (!window.active)
            GStatus.windowDisActived = true
    }

    onWidthChanged: {
        if(window.visibility!=Window.FullScreen && window.visibility !=Window.Maximized){
            FileControl.setSettingWidth(width)
        }

        GStatus.enableRatioAnimation = false
    }

    onHeightChanged: {
        if(window.visibility!=Window.FullScreen &&window.visibility!=Window.Maximized){
            FileControl.setSettingHeight(height)
        }
    }

    //关闭的时候保存信息
    onClosing: {
        FileControl.saveSetting()
        FileControl.terminateShortcutPanelProcess() //结束快捷键面板进程
    }

    GlobalVar{
        id: global
    }
    MenuItemStates {
        id: menuItemStates
    }
    FileDialog {
        id: importDialog
        title: qsTr("All photos and videos")
        currentFolder: shortcuts.pictures
        //selectMultiple: true
        nameFilters: albumControl.getAllFilters()
        onAccepted: {
            var bIsCustomAlbumImport = GStatus.currentViewType === Album.Types.ViewCustomAlbum && albumControl.isCustomAlbum(GStatus.currentCustomAlbumUId)
            //自定义相册不需要判重
            albumControl.importAllImagesAndVideosUrl(importDialog.fileUrls, GStatus.currentCustomAlbumUId, !bIsCustomAlbumImport)
        }
    }

    StackControl{
        id: stackControl
    }

    Album.EventGenerator {
        id: eventGenerator
    }

    Connections {
        target: albumControl
        function onSigActiveApplicationWindow() {
            window.requestActivate()
        }
    }

    Connections {
        function onCurrentSourceChanged() {
            window.title = FileControl.slotGetFileName(GControl.currentSource) + FileControl.slotFileSuffix(GControl.currentSource);
        }

        target: GControl
    }
}
