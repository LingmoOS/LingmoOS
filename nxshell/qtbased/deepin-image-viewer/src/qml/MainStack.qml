// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import org.deepin.dtk 1.0 as DTK
import org.deepin.image.viewer 1.0 as IV

Item {
    id: stackView

    // 打开图片对话框
    function openImageDialog() {
        if (Loader.Ready === fileDialogLoader.status) {
            fileDialogLoader.item.open();
        } else {
            fileDialogLoader.active = true;
        }
    }

    // 设置当前使用的图片源
    function setSourcePath(path) {
        if (IV.FileControl.isCurrentWatcherDir(path)) {
            // 更新当前文件路径
            IV.GControl.currentSource = path;
        } else {
            var sourcePaths = IV.FileControl.getDirImagePath(path);
            if (sourcePaths.length > 0) {
                IV.GControl.setImageFiles(sourcePaths, path);
                // 记录当前读取的图片信息
                IV.FileControl.resetImageFiles(sourcePaths);
                console.log("Load image info", path);
                switchImageView();
            } else {
                switchOpenImage();
            }
        }
    }

    function switchImageView() {
        IV.GStatus.stackPage = Number(IV.Types.ImageViewPage);
        contentLoader.setSource("qrc:/qml/FullImageView.qml");
    }

    function switchOpenImage() {
        IV.GStatus.stackPage = Number(IV.Types.OpenImagePage);
        window.title = "";
        contentLoader.setSource("qrc:/qml/OpenImageWidget.qml");
    }

    function switchSliderShow() {
        if (Number(IV.Types.ImageViewPage) === IV.GStatus.stackPage) {
            IV.GStatus.stackPage = Number(IV.Types.SliderShowPage);
            contentLoader.setSource("qrc:/qml/SliderShow.qml");
        }
    }

    anchors.fill: parent

    Component.onCompleted: {
        // main.cpp 从命令行启动时取得命令行参数，判断默认加载界面
        if (IV.GStatus.stackPage === Number(IV.Types.ImageViewPage)) {
            switchImageView();
        } else {
            switchOpenImage();
        }
    }

    Connections {
        // 关联外部通过 DBus 等方式触发调用看图
        function onOpenImageFile() {
            setSourcePath(fileName);
        }

        target: IV.FileControl
    }

    // 标题栏
    ViewTopTitle {
        id: titleRect

        z: parent.z + 1
    }

    // 展示内容
    Loader {
        id: contentLoader

        active: true
        anchors.fill: parent
    }

    DropArea {
        id: dropArea

        anchors.fill: parent

        onDropped: {
            if (drop.hasUrls && drop.urls.length !== 0) {
                setSourcePath(drop.urls[0]);
            }
        }
        onEntered: {
            background.color = "gray";
            drag.accept(Qt.CopyAction);
        }
        onExited: {
            background.color = "white";
            console.log("onExited");
        }
    }

    Loader {
        id: fileDialogLoader

        active: false
        asynchronous: true

        sourceComponent: FileDialog {
            id: fileDialog

            currentFolder: IV.FileControl.standardPicturesPath()
            fileMode: FileDialog.OpenFiles
            nameFilters: ["Image files (*.jpg *.png *.bmp *.gif *.ico *.jpe " + "*.jps *.jpeg *.jng *.koala *.koa *.lbm " + "*.iff *.mng *.pbm *.pbmraw *.pcd *.pcx " + "*.pgm *.pgmraw *.ppm *.ppmraw *.ras *.tga " + "*.targa *.tiff *.tif *.wbmp *.psd *.cut *.xbm " + "*.xpm *.dds *.fax *.g3 *.sgi *.exr *.pct *.pic " + "*.pict *.webp *.jxr *.mrw *.raf *.mef *.raw *.orf " + "*.djvu *.or2 *.icns *.dng *.svg *.nef *.pef *.pxm *.pnm)"]
            title: qsTr("Select pictures")

            Component.onCompleted: {
                fileDialog.open();
            }
            onAccepted: {
                stackView.setSourcePath(fileDialog.selectedFiles[0]);
            }
        }
    }

    // 快捷键打开帮助手册
    Shortcut {
        autoRepeat: false
        enabled: true
        sequence: "F1"

        onActivated: {
            DTK.ApplicationHelper.handleHelpAction();
        }
    }

    // 打开图片文件
    Shortcut {
        sequence: "Ctrl+O"

        onActivated: {
            // 不在动画展示状态
            if (Number(IV.Types.SliderShowPage) !== IV.GStatus.stackPage) {
                openImageDialog();
            }
        }
    }

    // show shortcut panel
    Shortcut {
        sequence: "Ctrl+Shift+/"

        onActivated: {
            var screenPos = mapToGlobal(parent.x, parent.y);
            IV.FileControl.showShortcutPanel(screenPos.x + parent.Window.width / 2, screenPos.y + parent.Window.height / 2);
        }
    }
}
