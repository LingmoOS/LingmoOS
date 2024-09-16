// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.deepin.dtk 1.0 as D
import org.deepin.dtk 1.0
import org.deepin.album 1.0 as Album

import "../"

DialogWindow {
    id: renamedialog
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint
    title: " "
    visible: false
    property bool isChangeView: false
    property bool importSelected: false
    minimumWidth: 400
    maximumWidth: 400
    minimumHeight: 190
    maximumHeight: 190

    width: 400
    height: 190

    icon : "deepin-album"

    signal sigCreateAlbumDone() //创建相册完成信号

    function setNormalEdit()
    {
        //重新设置焦点和名称
        nameedit.focus=true
        nameedit.text=qsTr("Unnamed")
    }

    Text {
        id: renametitle
        width: 308
        height: 24
        anchors {
            left: parent.left
            leftMargin: 46
            top: parent.top
        }
        font: DTK.fontManager.t5
        text: qsTr("New Album")
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignHCenter
    }
    Label{
        id:nameLabel
        width:42
        height: 36
        font: DTK.fontManager.t6
        anchors {
            left: parent.left
            leftMargin: 0
            top: nameedit.top
            topMargin:0
        }
        verticalAlignment: Text.AlignVCenter
        text:qsTr("Name:")
    }
    LineEdit {
        id: nameedit
        anchors {
            top: renametitle.bottom
            topMargin: 16
            left: parent.left
            leftMargin: 52
        }
        width: 328
        height: 36
        font: DTK.fontManager.t6
        focus: true
        maximumLength: 255
        validator: RegularExpressionValidator {regularExpression: /^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/ }
        text: qsTr("Unnamed")
        selectByMouse: true
//        alertText: qsTr("The file already exists, please use another name")
//        showAlert: FileControl.isShowToolTip(source,nameedit.text)
    }


    Button {
        id: cancelbtn
        anchors {
            top: nameedit.bottom
            topMargin: 15
            left: parent.left
            leftMargin: 0
        }
        text: qsTr("Cancel")
        width: 185
        height: 36
        font.pixelSize: 16
        onClicked: {
            renamedialog.visible = false
        }
    }

    RecommandButton {
        id: enterbtn
        anchors {
            top: nameedit.bottom
            topMargin: 14
            left: cancelbtn.right
            leftMargin: 10
        }
        text: qsTr("Confirm")
        enabled: nameedit.text !== "" ? true : false
        width: 185
        height: 38

        onClicked: {
            albumControl.createAlbum(nameedit.text )
            GStatus.albumChangeList = !GStatus.albumChangeList
            renamedialog.visible = false

            // 获取新相册index
            var index = albumControl.getAllCustomAlbumId().length - 1

            // 导入已选图片
            if (importSelected) {
                albumControl.insertIntoAlbum(albumControl.getAllCustomAlbumId()[index] , GStatus.selectedPaths)
            }

            // 切换到对应相册视图
            if (isChangeView) {
                GStatus.currentViewType = Album.Types.ViewCustomAlbum
                GStatus.currentCustomAlbumUId = albumControl.getAllCustomAlbumId()[index]
                sigCreateAlbumDone()
            }

            //侧边栏如果是关闭状态，侧边栏会自动打开
            if (leftSidebar.x !== 0) {
                showSliderAnimation.start()
            }
        }
    }

    onVisibleChanged: {
        setX(window.x  + window.width / 2 - width / 2)
        setY(window.y  + window.height / 2 - height / 2)
    }
}
