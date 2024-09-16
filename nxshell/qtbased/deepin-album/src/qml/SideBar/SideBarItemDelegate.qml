// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS

ItemDelegate {
    id: item
    MouseArea {
        anchors.fill: item
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        onDoubleClicked:{
            item.rename()
        }
        onClicked: (mouse)=> {
            sideListView.currentIndex = index
            item.forceActiveFocus()
            control.itemClicked(model.uuid, model.displayName)
            item.checked = true
            if(mouse.button === Qt.RightButton){
                control.itemRightClicked(model.uuid, model.displayName)
            }
        }
    }

    RoundRectangle {
        visible: hovered
        anchors.fill: parent
        color: DTK.themeType === ApplicationHelper.LightType ? Qt.rgba(0,0,0,0.1)
                                                             : Qt.rgba(1,1,1,0.1)
        radius: DS.Style.control.radius
        corners: item.corners
    }

    DciIcon {
        id: siderIcon
        anchors {
            left: item.left
            leftMargin: 10
            verticalCenter: item.verticalCenter
        }
        name: model.icon
        palette: DTK.makeIconPalette(item.palette)
        sourceSize: Qt.size(20, 20)
    }

    Label {
        id: songName
        width: 100;
        anchors {
            left: siderIcon.right
            leftMargin: 10
            verticalCenter: item.verticalCenter
        }
        font: DTK.fontManager.t6
        text: "%1".arg(model.displayName)
        elide: Text.ElideRight
    }
    // 移除设备按钮，仅在设备列表显示
    ActionButton {
        visible: control.showRemoveDeviceBtn
        anchors {
            left: songName.right
            leftMargin: 10
            verticalCenter: item.verticalCenter
        }
        width: 10
        height:  10
        icon.name: "arrow"
        icon.width: 10
        icon.height: 10
        onClicked:{
            control.removeDeviceBtnClicked(index)
        }
    }
    LineEdit {
        id: keyLineEdit
        width: 180
        visible: false;
        maximumLength: 30
        validator: RegularExpressionValidator {regularExpression: /^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/ }
        onEditingFinished: {
            item.checked = true;
            songName.visible = true;
            siderIcon.visible = true;
            keyLineEdit.visible = false;
            if (keyLineEdit.text !== "" && albumControl.renameAlbum(albumControl.getAllCustomAlbumId()[index], keyLineEdit.text)) {
                songName.text = keyLineEdit.text

                // 通知自定义相册视图刷新相册名称
                GStatus.sigCustomAlbumNameChaged(GStatus.currentCustomAlbumUId, songName.text)
            }
        }
        onActiveFocusChanged: {
            //EventsFilter.setEnabled(!activeFocus)
        }
    }

    onCheckedChanged: {
        control.itemCheckedChanged(index, item.checked)
    }

    function enableRename(){
        item.checked = true;
        keyLineEdit.text = songName.text;
        keyLineEdit.forceActiveFocus()
        songName.visible = false;
        siderIcon.visible = false;
        keyLineEdit.visible = true;
        item.checked = false;
    }
    function rename(){
        if(!model.editable)
            return;
        enableRename();
        keyLineEdit.selectAll();
    }
    function switchToPrevious(){
        item.forceActiveFocus();
        control.itemClicked(model.uuid, model.displayName);
        item.checked = true
    }

    // 屏蔽空格响应
    Keys.onSpacePressed: (event)=> { event.accepted=false; }
    Keys.onReleased: (event)=> { event.accepted=(event.key===Qt.Key_Space); }
}
