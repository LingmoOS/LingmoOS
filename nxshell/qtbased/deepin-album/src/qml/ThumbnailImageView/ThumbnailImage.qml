// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml.Models
import QtQuick.Window
import QtQuick.Controls
import org.deepin.dtk 1.0 as D
import org.deepin.album 1.0 as Album

import "./CollecttionView"
import "./HaveImportedView"
import "./RecentlyDeletedView"
import "./CustomAlbum"
import "./DeviceAlbum"
import "./"

import "./../Control"
import "./../PreviewImageViewer/InformationDialog"
import "./../"
//本文件用于替代stackwidget的作用，通过改变global的0-n来切换窗口

Item {

    signal escKeyPressed()

    CollecttionView{
        id: collecttionView
        show: GStatus.currentViewType === Album.Types.ViewCollecttion
    }
    HaveImportedView{
        show: GStatus.currentViewType === Album.Types.ViewHaveImported
    }
    CustomAlbum{
        show: GStatus.currentViewType === Album.Types.ViewFavorite
    }
    RecentlyDeletedView{
        show: GStatus.currentViewType === Album.Types.ViewRecentlyDeleted
    }
    CustomAlbum{
        show: GStatus.currentViewType === Album.Types.ViewCustomAlbum
    }
    SearchView{
        show: GStatus.currentViewType === Album.Types.ViewSearchResult
    }
    DeviceAlbum{
        show: GStatus.currentViewType === Album.Types.ViewDevice
    }

    //export窗口
    ExportDialog {
        id: exportdig
    }

    EmptyWarningDialog {
        id: emptyWarningDig
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

    Connections {
        target: titleAlubmRect
        function onCollectionBtnClicked(index) {
            if (GStatus.currentCollecttionViewIndex === index)
                return
            // 点击按钮，动画切换类型设定为翻页滚动
            GStatus.currentSwitchType = Album.Types.FlipScroll
            collecttionView.setIndex(index)
        }
    }

    Shortcut {
        enabled: GStatus.stackControlCurrent === 0
        autoRepeat: false
        sequence: "Esc"
        onActivated: {
            escKeyPressed()
        }
    }

    Shortcut {
        enabled: true
        autoRepeat: false
        sequence : "Ctrl+A"
        onActivated : {
            GStatus.sigSelectAll(true)
        }
    }

    Shortcut {
        enabled: visible && menuItemStates.canCopy
        autoRepeat: false
        sequence : "Ctrl+C"
        onActivated : {
            FileControl.copyImage(GStatus.selectedPaths)
        }
    }

    Shortcut {
        enabled: visible && menuItemStates.canDelete && GStatus.currentViewType !== Album.Types.ViewDevice
        autoRepeat: false
        sequence : "Delete"
        onActivated : {          
            if (menuItemStates.isInTrash) {
                deleteDialog.setDisplay(menuItemStates.isInTrash ? Album.Types.TrashSel : Album.Types.TrashNormal, GStatus.selectedPaths.length)
                deleteDialog.show()
            } else {
                deleteDialog.deleteDirectly()
            }
        }
    }

    Shortcut {
        enabled: visible && menuItemStates.canRotate && GStatus.currentViewType !== Album.Types.ViewDevice
        autoRepeat: false
        sequence : "Ctrl+R"
        onActivated : {
            FileControl.rotateFile(GStatus.selectedPaths, 90)
        }
    }

    Shortcut {
        enabled: visible && menuItemStates.canRotate && GStatus.currentViewType !== Album.Types.ViewDevice
        autoRepeat: false
        sequence : "Ctrl+Shift+R"
        onActivated : {
            FileControl.rotateFile(GStatus.selectedPaths, -90)
        }
    }

    Shortcut {
        enabled: visible
        autoRepeat: true
        sequence : "Page Up"
        onActivated : {
            GStatus.sigPageUp()
        }
    }

    Shortcut {
        enabled: visible
        autoRepeat: true
        sequence : "Page Down"
        onActivated : {
            GStatus.sigPageDown()
        }
    }

    Shortcut {
        enabled: visible
        autoRepeat: false
        sequence : "Ctrl+="
        onActivated : {
            var curValue = statusBar.sliderValue
            statusBar.setSliderWidgetValue(curValue + 1)
        }
    }

    Shortcut {
        enabled: visible
        autoRepeat: false
        sequence : "Ctrl+-"
        onActivated : {
            var curValue = statusBar.sliderValue
            statusBar.setSliderWidgetValue(curValue - 1)
        }
    }

    Shortcut {
        enabled: visible && leftSidebar.visible
        autoRepeat: false
        sequence: "Ctrl+Shift+N"
        onActivated: {
            newAlbum.setNormalEdit()
            newAlbum.isChangeView = true
            newAlbum.show()
            forceActiveFocus()
        }
    }

    Shortcut {
        enabled: visible
        autoRepeat: false
        sequence: "F1"
        onActivated: {
            D.ApplicationHelper.handleHelpAction()
        }
    }

    Shortcut {
        enabled: visible
        autoRepeat: false
        sequence: "Ctrl+Shift+/"
        onActivated: {
            albumControl.ctrlShiftSlashShortcut(window.x, window.y, window.width, window.height)
        }
    }
}
