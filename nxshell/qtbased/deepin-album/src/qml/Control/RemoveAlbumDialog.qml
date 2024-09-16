// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.deepin.dtk 1.0

DialogWindow {
    id: deleteDialog
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint
    title: " "
    visible: false

    minimumWidth: 400
    maximumWidth: 400
    minimumHeight: 140
    maximumHeight: 140

    width: 400
    height: 140

    icon : "deepin-album"

    signal sigDoRemoveAlbum(int type)

    property int deleteType: 0

    Text {
        id: deleteTitle
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        font: DTK.fontManager.t5
        text: qsTr("Are you sure you want to delete this album?")
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignHCenter
    }

    Button {
        id: cancelbtn
        anchors {
            top: deleteTitle.bottom
            topMargin: 15
            left: parent.left
            leftMargin: 0
        }
        text: qsTr("Cancel")
        width: 185
        height: 36
        font.pixelSize: 16
        onClicked: {
            deleteDialog.visible = false
        }
    }

    WarningButton {
        id: enterbtn
        anchors {
            top: deleteTitle.bottom
            topMargin: 15
            left: cancelbtn.right
            leftMargin: 10
        }
        text: qsTr("Delete")
        width: 185
        height: 36

        onClicked: {
            deleteDialog.visible = false
            sigDoRemoveAlbum(deleteType)
        }
    }

    onVisibleChanged: {
        setX(window.x + window.width / 2 - width / 2)
        setY(window.y + window.height / 2 - height / 2)
    }
}
