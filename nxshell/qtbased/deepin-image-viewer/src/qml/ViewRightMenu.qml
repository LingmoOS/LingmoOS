// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV
import "./Utils"

Menu {
    id: optionMenu

    // 处理拷贝快捷键冲突
    property bool copyableConfig: true
    property bool deletable: !isNullImage && IV.FileControl.isCanDelete(imageSource)
    property url imageSource: IV.GControl.currentSource
    property bool isNullImage: imageInfo.type === IV.Types.NullImage
    property bool readable: !isNullImage && IV.FileControl.isCanReadable(imageSource)
    property bool renamable: !isNullImage && IV.FileControl.isCanRename(imageSource)
    property bool rotatable: !isNullImage && IV.FileControl.isRotatable(imageSource)
    property bool supportOcr: !isNullImage && IV.FileControl.isCanSupportOcr(imageSource)
    property bool supportWallpaper: !isNullImage && IV.FileControl.isSupportSetWallpaper(imageSource)

    maxVisibleItems: 20
    x: 250
    y: 600

    RightMenuItem {
        id: rightFullscreen

        function switchFullScreen() {
            IV.GStatus.showFullScreen = !IV.GStatus.showFullScreen;
        }

        text: !window.isFullScreen ? qsTr("Fullscreen") : qsTr("Exit fullscreen")

        onTriggered: switchFullScreen()

        Shortcut {
            sequence: "F11"

            onActivated: rightFullscreen.switchFullScreen()
        }

        Shortcut {
            enabled: window.isFullScreen
            sequence: "Esc"

            onActivated: rightFullscreen.switchFullScreen()
        }
    }

    RightMenuItem {
        text: qsTr("Print")
        visible: !isNullImage

        onTriggered: {
            optionMenu.close();
            IV.FileControl.showPrintDialog(imageSource);
        }

        Shortcut {
            sequence: "Ctrl+P"

            onActivated: {
                optionMenu.close();
                IV.FileControl.showPrintDialog(imageSource);
            }
        }
    }

    RightMenuItem {
        text: qsTr("Extract text")
        visible: supportOcr

        onTriggered: {
            IV.GControl.submitImageChangeImmediately();
            IV.FileControl.ocrImage(imageSource, IV.GControl.currentFrameIndex);
        }

        Shortcut {
            enabled: supportOcr
            sequence: "Alt+O"

            onActivated: {
                IV.GControl.submitImageChangeImmediately();
                IV.FileControl.ocrImage(imageSource, IV.GControl.currentFrameIndex);
            }
        }
    }

    RightMenuItem {
        text: qsTr("Slide show")

        onTriggered: {
            stackView.switchSliderShow();
        }

        Shortcut {
            sequence: "F5"

            onActivated: {
                stackView.switchSliderShow();
            }
        }
    }

    MenuSeparator {
        id: firstSeparator

    }

    RightMenuItem {
        text: qsTr("Copy")
        visible: readable

        onTriggered: {
            IV.GControl.submitImageChangeImmediately();
            IV.FileControl.copyImage(imageSource);
        }

        Shortcut {
            enabled: readable && copyableConfig
            sequence: "Ctrl+C"

            onActivated: {
                IV.GControl.submitImageChangeImmediately();
                IV.FileControl.copyImage(imageSource);
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
                renamedialog.show();
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
            enabled: deletable && parent.enabled
            sequence: "Delete"

            onActivated: {
                thumbnailViewBackGround.deleteCurrentImage();
            }
        }
    }

    // 不允许无读写权限时上方选项已屏蔽，不展示此分割条
    MenuSeparator {
        // 不显示分割条时调整高度，防止菜单项间距不齐
        height: visible ? firstSeparator.height : 0
        visible: IV.FileControl.isCanReadable(imageSource) || IV.FileControl.isCanDelete(imageSource)
    }

    RightMenuItem {
        id: rotateClockItem

        text: qsTr("Rotate clockwise")
        visible: rotatable

        onTriggered: {
            imageViewer.rotateImage(90);
        }

        Shortcut {
            enabled: rotatable
            sequence: "Ctrl+R"

            onActivated: {
                imageViewer.rotateImage(90);
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
            enabled: rotatable
            sequence: "Ctrl+Shift+R"

            onActivated: {
                imageViewer.rotateImage(-90);
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

        enabled: visible && window.height > IV.GStatus.minHideHeight && window.width > IV.GStatus.minWidth
        text: !IV.GStatus.enableNavigation ? qsTr("Show navigation window") : qsTr("Hide navigation window")
        visible: !isNullImage

        onTriggered: {
            if (!parent.enabled) {
                return;
            }
            IV.GStatus.enableNavigation = !IV.GStatus.enableNavigation;
        }
    }

    RightMenuItem {
        text: qsTr("Set as wallpaper")
        visible: supportWallpaper

        onTriggered: {
            IV.GControl.submitImageChangeImmediately();
            IV.FileControl.setWallpaper(imageSource);
        }

        Shortcut {
            enabled: supportWallpaper
            sequence: "Ctrl+F9"

            onActivated: {
                IV.GControl.submitImageChangeImmediately();
                IV.FileControl.setWallpaper(imageSource);
            }
        }
    }

    RightMenuItem {
        text: qsTr("Display in file manager")

        onTriggered: {
            IV.FileControl.displayinFileManager(imageSource);
        }

        Shortcut {
            enabled: parent.visible
            sequence: "Alt+D"

            onActivated: {
                IV.FileControl.displayinFileManager(imageSource);
            }
        }
    }

    RightMenuItem {
        text: qsTr("Image info")

        onTriggered: {
            infomationDig.show();
        }

        Shortcut {
            enabled: parent.visible
            sequence: "Ctrl+I"

            onActivated: {
                infomationDig.show();
            }
        }
    }

    IV.ImageInfo {
        id: imageInfo

        source: imageSource

        onStatusChanged: {
            if (IV.ImageInfo.Ready === imageInfo.status) {
                isNullImage = (imageInfo.type === IV.Types.NullImage);
            } else {
                isNullImage = true;
            }
        }
    }
}
