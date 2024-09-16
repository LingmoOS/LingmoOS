// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV
import org.deepin.album 1.0 as Album
import "./Utils"

Menu {
    id: optionMenu

    // 处理拷贝快捷键冲突
    property bool copyableConfig: true
    property bool deletable: !isNullImage && FileControl.isCanDelete(imageSource.toString())
    property url imageSource: GControl.currentSource
    property bool isNullImage: imageInfo.type === Album.Types.NullImage
    property bool readable: !isNullImage && FileControl.isCanReadable(imageSource)
    property bool renamable: !isNullImage && FileControl.isCanRename(imageSource)
    property bool rotatable: !isNullImage && FileControl.isRotatable(imageSource.toString())
    property bool supportOcr: !isNullImage && FileControl.isCanSupportOcr(imageSource)
    property bool supportWallpaper: !isNullImage && FileControl.isSupportSetWallpaper(imageSource)
    property bool canExport: FileControl.pathExists(imageSource) && FileControl.isImage(imageSource) && !FileControl.isVideo(imageSource)

    maxVisibleItems: 20
    x: 250
    y: 600

    RightMenuItem {
        id: rightFullscreen

        function switchFullScreen() {
            GStatus.showFullScreen = !GStatus.showFullScreen;
        }

        text: !window.isFullScreen ? qsTr("Fullscreen") : qsTr("Exit fullscreen")

        onTriggered: switchFullScreen()

        Shortcut {
            enabled: stackView.visible
            sequence: "F11"

            onActivated: rightFullscreen.switchFullScreen()
        }

        Shortcut {
            enabled: (window.isFullScreen && stackView.visible && GStatus.stackPage !== Number(Album.Types.SliderShowPage)) ? true : false
            sequence: "Esc"

            onActivated: rightFullscreen.switchFullScreen()
        }
    }

    RightMenuItem {
        text: qsTr("Print")
        visible: !isNullImage

        onTriggered: {
            optionMenu.close();
            FileControl.showPrintDialog(imageSource);
        }

        Shortcut {
            sequence: "Ctrl+P"
            enabled: !isNullImage && stackView.visible
            onActivated: {
                optionMenu.close();
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    FileControl.showPrintDialog(imageSource);
                }
            }
        }
    }

    RightMenuItem {
        text: qsTr("Extract text")
        visible: supportOcr

        onTriggered: {
            GControl.submitImageChangeImmediately();
            FileControl.ocrImage(imageSource, GControl.currentFrameIndex);
        }

        Shortcut {
            enabled: supportOcr
            sequence: "Alt+O"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    GControl.submitImageChangeImmediately();
                    FileControl.ocrImage(imageSource, GControl.currentFrameIndex);
                }
            }
        }
    }

    RightMenuItem {
        text: qsTr("Slide show")

        onTriggered: {
            stackView.switchSliderShow();
        }

        Shortcut {
            enabled: stackView.visible
            sequence: "F5"

            onActivated: {
                if (parent.visible && Number(Album.Types.OpenImagePage) !== GStatus.stackPage) {
                    stackView.switchSliderShow();
                }
            }
        }
    }

    MenuSeparator {
        id: firstSeparator

    }

    //导出图片为其它格式
    RightMenuItem {
        text: qsTr("Export")
        visible: canExport && !menuItemStates.isInTrash && FileControl.isAlbum()
        onTriggered: {
            excuteExport()
        }

        Shortcut {
            enabled: stackView.visible && canExport && !menuItemStates.isInTrash && FileControl.isAlbum()
            sequence : "Ctrl+E"
            onActivated : {
               excuteExport()
            }
        }
    }

    RightMenuItem {
        text: qsTr("Copy")
        visible: readable

        onTriggered: {
            if (parent.visible) {
                GControl.submitImageChangeImmediately();
                FileControl.copyImage(imageSource);
            }
        }

        Shortcut {
            enabled: readable && copyableConfig && stackView.visible
            sequence: "Ctrl+C"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    GControl.submitImageChangeImmediately();
                    FileControl.copyImage(imageSource);
                }
            }
        }
    }

    RightMenuItem {
        text: qsTr("Rename")
        visible: renamable

        onTriggered: {
            renamedialog.show();
        }

        Shortcut {
            enabled: renamable
            sequence: "F2"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    renamedialog.show();
                }
            }
        }
    }

    RightMenuItem {
        enabled: !thumbnailViewBackGround.imageDeleting
        text: qsTr("Delete")
        visible: deletable

        onTriggered: {
            thumbnailViewBackGround.deleteCurrentImage();
        }

        Shortcut {
            enabled: deletable && parent.enabled && stackView.visible
            sequence: "Delete"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    thumbnailViewBackGround.deleteCurrentImage();
                }
            }
        }
    }

    //分割条-收藏
    MenuSeparator {
        visible: !menuItemStates.isInTrash && FileControl.isAlbum()
        // 不显示分割条时调整高度，防止菜单项间距不齐
        height: visible ? firstSeparator.height : 0
    }

    //添加到我的收藏
    RightMenuItem {
        id: favoriteAction
        text: qsTr("Favorite")
        visible: !menuItemStates.isInTrash && imageViewer.canFavorite && FileControl.isAlbum()
        onTriggered: {
            imageViewer.executeFavorite()
        }
    }

    //从我的收藏中移除
    RightMenuItem {
        id: unFavoriteAction
        text: qsTr("Unfavorite")
        visible: !menuItemStates.isInTrash && !imageViewer.canFavorite && FileControl.isAlbum()
        onTriggered: {
            imageViewer.executeUnFavorite()
        }
    }

    // 不允许无读写权限时上方选项已屏蔽，不展示此分割条
    MenuSeparator {
        // 不显示分割条时调整高度，防止菜单项间距不齐
        height: visible ? firstSeparator.height : 0
        visible: FileControl.isCanReadable(imageSource.toString()) || FileControl.isCanDelete(imageSource.toString())
    }

    RightMenuItem {
        id: rotateClockItem

        text: qsTr("Rotate clockwise")
        visible: rotatable

        onTriggered: {
            imageViewer.rotateImage(90);
        }

        Shortcut {
            enabled: rotatable && stackView.visible
            sequence: "Ctrl+R"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    imageViewer.rotateImage(90);
                }
            }
        }
    }

    RightMenuItem {
        id: rotateCounterClockItem

        text: qsTr("Rotate counterclockwise")
        visible: rotatable

        onTriggered: {
            imageViewer.rotateImage(-90);
        }

        Shortcut {
            enabled: rotatable && stackView.visible
            sequence: "Ctrl+Shift+R"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    imageViewer.rotateImage(-90);
                }
            }
        }
    }

    // 不允许无读写权限时上方选项已屏蔽，不展示此分割条
    MenuSeparator {
        // 不显示分割条时调整高度，防止菜单项间距不齐
        height: visible ? firstSeparator.height : 0
        visible: rotateClockItem.visible || rotateCounterClockItem.visible
    }

    RightMenuItem {
        id: enableNavigation

        enabled: visible && window.height > GStatus.minHideHeight && window.width > GStatus.minWidth
        text: !GStatus.enableNavigation ? qsTr("Show navigation window") : qsTr("Hide navigation window")
        visible: !isNullImage

        onTriggered: {
            if (!parent.enabled) {
                return;
            }
            GStatus.enableNavigation = !GStatus.enableNavigation;
        }
    }

    RightMenuItem {
        text: qsTr("Set as wallpaper")
        visible: supportWallpaper

        onTriggered: {
            GControl.submitImageChangeImmediately();
            FileControl.setWallpaper(imageSource);
        }

        Shortcut {
            enabled: supportWallpaper && stackView.visible
            sequence: "Ctrl+F9"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    GControl.submitImageChangeImmediately();
                    FileControl.setWallpaper(imageSource);
                }
            }
        }
    }

    RightMenuItem {
        text: qsTr("Display in file manager")

        onTriggered: {
            FileControl.displayinFileManager(imageSource);
        }

        Shortcut {
            enabled: stackView.visible
            sequence: "Alt+D"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    FileControl.displayinFileManager(imageSource);
                }
            }
        }
    }

    RightMenuItem {
        text: qsTr("Image info")

        onTriggered: {
            infomationDig.show();
        }

        Shortcut {
            enabled: stackView.visible
            sequence: "Ctrl+I"

            onActivated: {
                if (parent.visible && Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
                    infomationDig.show();
                }
            }
        }
    }

    IV.ImageInfo {
        id: imageInfo

        source: imageSource

        onStatusChanged: {
            if (IV.ImageInfo.Ready === imageInfo.status) {
                isNullImage = (imageInfo.type === Album.Types.NullImage);
            } else {
                isNullImage = true;
            }
        }
    }
}
