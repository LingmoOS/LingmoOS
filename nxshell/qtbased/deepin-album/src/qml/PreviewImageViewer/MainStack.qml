// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import org.deepin.dtk 1.0 as D
import org.deepin.image.viewer 1.0 as IV
import org.deepin.album 1.0 as Album

import "../Control/Animation"

FadeInoutAnimation {
    id: stackView

    property alias iconName: titleRect.iconName

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
        if (FileControl.isCurrentWatcherDir(path)) {
            // 更新当前文件路径
            GControl.currentSource = path;
        } else {
            var sourcePaths = FileControl.getDirImagePath(path);
            if (sourcePaths.length > 0) {
                GControl.setImageFiles(sourcePaths, path);
                // 记录当前读取的图片信息
                FileControl.resetImageFiles(sourcePaths);
                console.log("Load image info", path);
                switchImageView();
            } else {
                switchOpenImage();
            }
        }
    }

    function switchImageView() {
        GStatus.stackPage = Number(Album.Types.ImageViewPage);
        window.title = FileControl.slotGetFileName(GControl.currentSource) + FileControl.slotFileSuffix(GControl.currentSource)
        contentLoader.setSource("qrc:/qml/PreviewImageViewer/FullImageView.qml");
    }

    function switchOpenImage() {
        GStatus.stackPage = Number(Album.Types.OpenImagePage);
        window.title = "";
        contentLoader.setSource("qrc:/qml/PreviewImageViewer/OpenImageWidget.qml");
    }

    function switchSliderShow() {
        if (Number(Album.Types.ImageViewPage) === GStatus.stackPage) {
            GStatus.stackPage = Number(Album.Types.SliderShowPage);
            contentLoader.setSource("qrc:/qml/PreviewImageViewer/SliderShow.qml");
        }
    }

    anchors.fill: parent

    Component.onCompleted: {
        // main.cpp 从命令行启动时取得命令行参数，判断默认加载界面
        if (GStatus.stackPage === Number(Album.Types.ImageViewPage)) {
            switchImageView();
        } else {
            switchOpenImage();
        }
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

            currentFolder: shortcuts.pictures
            // selectMultiple: true
            fileMode: FileDialog.OpenFiles
            nameFilters: ["Image files (*.jpg *.png *.bmp *.gif *.ico *.jpe " + "*.jps *.jpeg *.jng *.koala *.koa *.lbm " + "*.iff *.mng *.pbm *.pbmraw *.pcd *.pcx " + "*.pgm *.pgmraw *.ppm *.ppmraw *.ras *.tga " + "*.targa *.tiff *.tif *.wbmp *.psd *.cut *.xbm " + "*.xpm *.dds *.fax *.g3 *.sgi *.exr *.pct *.pic " + "*.pict *.webp *.jxr *.mrw *.raf *.mef *.raw *.orf " + "*.djvu *.or2 *.icns *.dng *.svg *.nef *.pef *.pxm *.pnm)"]
            title: qsTr("Select pictures")

            Component.onCompleted: {
                fileDialog.open();
            }
            onAccepted: {
                var path = fileDialog.fileUrls[0]
                stackView.setSourcePath(path);
                if (FileControl.isAlbum() && GControl.currentSource.toString() !== "") {
                    var sourcePaths = FileControl.getDirImagePath(path);
                    albumControl.importAllImagesAndVideos(sourcePaths);
                }
            }
        }
    }

    // 快捷键打开帮助手册
    Shortcut {
        autoRepeat: false
        enabled: stackView.visible
        sequence: "F1"

        onActivated: {
            D.ApplicationHelper.handleHelpAction();
        }
    }

    // 打开图片文件
    Shortcut {
        enabled: Number(Album.Types.SliderShowPage) !== GStatus.stackPage && stackView.visible
        sequence: "Ctrl+O"

        onActivated: {
            // 不在动画展示状态
            if (Number(Album.Types.SliderShowPage) !== GStatus.stackPage) {
                openImageDialog();
            }
        }
    }
}
