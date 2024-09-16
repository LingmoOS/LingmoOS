// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.deepin.dtk 1.0
import org.deepin.album 1.0 as Album

import "../"

DialogWindow {
    id: deleteDialog
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint
    title: " "
    visible: false

    minimumWidth: 400
    maximumWidth: 400
    minimumHeight: 160
    maximumHeight: 160

    width: 400
    height: 160

    header: DialogTitleBar {
        enableInWindowBlendBlur: true
        icon {
            mode: DTK.NormalState
            name: "deepin-album"
        }
    }

    property int type: 0

    signal sigDoDeleteImg()
    signal sigDoAllDeleteImg()

    function setDisplay(deltype, count) {
        type = deltype
        if(deltype === Album.Types.TrashNormal) {
            if(count === 1) {
                textMetics.text = qsTr("Are you sure you want to delete this file locally?")
                deleteTips.text  = qsTr("You can restore it in the trash")
            } else {
                textMetics.text = qsTr("Are you sure you want to delete %1 files locally?").arg(count)
                deleteTips.text  = qsTr("You can restore them in the trash")
            }
        } else {
            if(count === 1) {
                textMetics.text = qsTr("Are you sure you want to permanently delete this file?")
                deleteTips.text  = qsTr("You cannot restore it any longer")
            } else {
                textMetics.text = qsTr("Are you sure you want to permanently delete %1 files?").arg(count)
                deleteTips.text  = qsTr("You cannot restore them any longer")
            }
        }
    }

    //不显示弹窗直接删除
    function deleteDirectly() {
        sigDoDeleteImg()
    }

    Label {
        id: deleteTitle
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        property Palette textColor: Palette {
            normal: ("black")
            normalDark: ("white")
        }

        color: ColorSelector.textColor
        font: DTK.fontManager.t5
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        text: textMetics.elidedText
        TextMetrics {
            id: textMetics
            elide: Text.ElideRight
            elideWidth: 400
            font: deleteTitle.font
        }
    }

    Label {
        id: deleteTips
        opacity: 0.7
        anchors {
            top: deleteTitle.bottom
            horizontalCenter: deleteTitle.horizontalCenter
        }
        font: DTK.fontManager.t6
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignHCenter
    }

    Button {
        id: cancelbtn
        anchors {
            top: deleteTips.bottom
            topMargin: 15
            left: parent.left
            leftMargin: 0
        }
        text: qsTr("Cancel")
        width: 185
        height: 36
        font: DTK.fontManager.t6
        onClicked: {
            deleteDialog.visible = false
        }
    }

    WarningButton {
        id: enterbtn
        anchors {
            top: deleteTips.bottom
            topMargin: 15
            left: cancelbtn.right
            leftMargin: 10
        }
        text: qsTr("Delete")
        width: 185
        height: 36
        font: DTK.fontManager.t6

        onClicked: {
            deleteDialog.visible = false
            switch (type) {
                case Album.Types.TrashNormal:
                case Album.Types.TrashSel:
                    sigDoDeleteImg()
                    break
                case Album.Types.TrashAll:
                    sigDoAllDeleteImg()
                    break
            }

        }
    }

    onVisibleChanged: {
        setX(window.x + window.width / 2 - width / 2)
        setY(window.y + window.height / 2 - height / 2)
    }
}
