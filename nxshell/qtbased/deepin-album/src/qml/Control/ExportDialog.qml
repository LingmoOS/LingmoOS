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

DialogWindow {

    id: exportdialog
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint
    title: " "
    visible: false

    minimumWidth: 400
    maximumWidth: 400
    minimumHeight: 320
    maximumHeight: 320

    width: 400
    height: 320

    property string filePath: ""
    property string saveName: nameedit.text
    property int saveIndex: saveCombox.currentIndex
    property string savefileFormat: formatCombox.displayText
    property int pictureQuality: piczSlider.value
    property string saveFolder: ""
    property var messageToId

    property int labelWidth: 80
    property int lineEditWidth: 300
    property int btnWidth: 185
    property int btnHeight: 36

    icon : "deepin-album"

    Text {
        id: exporttitle
        width: 308
        height: 24
        anchors {
            left: parent.left
            leftMargin: 46
            top: parent.top
        }
        font: DTK.fontManager.t5
        text: qsTr("Export")
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignHCenter
    }

    Label{
        id:nameLabel
        width: labelWidth
        height: btnHeight
        font: DTK.fontManager.t6
        anchors {
            left: parent.left
            leftMargin: 0
            top: nameedit.top
            topMargin:0
        }
        verticalAlignment: Text.AlignVCenter
        text: qsTr("Name:")
    }
    LineEdit {
        id: nameedit
        anchors {
            top: exporttitle.bottom
            topMargin: 16
            left: nameLabel.right
        }
        clearButton.visible: activeFocus
        width: lineEditWidth
        height: btnHeight
        font: DTK.fontManager.t6
        focus: true
        maximumLength: 255
        validator: RegularExpressionValidator {regularExpression: /^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/ }
        text: ""
        selectByMouse: true
    }

    Label{
        id: saveLabel
        width: labelWidth
        height: btnHeight
        font: DTK.fontManager.t6
        anchors {
            left: parent.left
            leftMargin: 0
            top: nameLabel.bottom
            topMargin:10
        }
        verticalAlignment: Text.AlignVCenter
        text:qsTr("Save to:")
    }

    ComboBox{
        id: saveCombox
        width: lineEditWidth
        height: btnHeight
        font: DTK.fontManager.t6
        anchors {
            top: nameedit.bottom
            topMargin: 10
            left: saveLabel.right
        }

        textRole: "key"
        model: ListModel {
            ListElement { key: qsTr("Pictures"); value: 0 }
            ListElement { key: qsTr("Documents"); value: 1 }
            ListElement { key: qsTr("Downloads"); value: 2 }
            ListElement { key: qsTr("Desktop"); value: 3 }
            ListElement { key: qsTr("Videos"); value: 4 }
            ListElement { key: qsTr("Music"); value: 5 }
            ListElement { key: qsTr("Select other directories"); value: 6 }
        }
        delegate:    MenuItem {
            text: key
            width: parent.width
            height : btnHeight
            highlighted: saveCombox.highlightedIndex === index
            onClicked: {
                saveIndex = value
                if(value == 6){

                    exportdialog.flags = Qt.Window
                    var tmpFolder = albumControl.getFolder();
                    if(tmpFolder.length > 0) {
                        key = tmpFolder
                    } else {
                        key = qsTr("Select other directories")
                    }
                    exportdialog.flags = Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint

                    saveFolder = key
                }
            }
        }
    }

    Label{
        id: fileFormatLabel
        width: labelWidth
        height: btnHeight
        font: DTK.fontManager.t6
        anchors {
            left: parent.left
            leftMargin: 0
            top: saveLabel.bottom
            topMargin:10
        }
        verticalAlignment: Text.AlignVCenter
        text: qsTr("Format:")
    }

    ComboBox{
        id :formatCombox
        width: lineEditWidth
        height: btnHeight
        font: DTK.fontManager.t6
        anchors {
            top: saveCombox.bottom
            topMargin: 10
            left: fileFormatLabel.right
        }
        model :albumControl.imageCanExportFormat(filePath).length
        displayText: albumControl.imageCanExportFormat(filePath)[currentIndex]
        delegate:    MenuItem {
            text: albumControl.imageCanExportFormat(filePath)[index]
            width: parent.width
            height: btnHeight
            highlighted: formatCombox.highlightedIndex === index
            onClicked: {

            }
        }
    }

    Label{
        id: piczLabel
        width: labelWidth
        height: btnHeight
        font: DTK.fontManager.t6
        anchors {
            left: parent.left
            leftMargin: 0
            top: fileFormatLabel.bottom
            topMargin:10
        }
        verticalAlignment: Text.AlignVCenter
        text:qsTr("Quality:")
    }

    Slider{
        id: piczSlider
        width: 246
        height: btnHeight
        font: DTK.fontManager.t6
        anchors {
            top: formatCombox.bottom
            topMargin: 16
            left: piczLabel.right
        }
        from: 1
        value: 100
        to: 100
        stepSize:1
    }

    Label{
        id: bfLable
        height: btnHeight
        font: DTK.fontManager.t7
        color: "#7C7C7C"
        anchors {
            top: formatCombox.bottom
            topMargin: 10
            left: piczSlider.right
            right: parent.right
            rightMargin: 10
        }
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        text: piczSlider.value + "%"
    }

    Button {
        id: cancelbtn
        anchors {
            top: bfLable.bottom
            topMargin: 9
            left: parent.left
            leftMargin: 0
        }
        text: qsTr("Cancel")
        width: btnWidth
        height: btnHeight
        font: DTK.fontManager.t6
        onClicked: {
            exportdialog.visible = false
        }
    }

    RecommandButton {
        id: enterbtn
        anchors {
            top: bfLable.bottom
            topMargin: 8
            left: cancelbtn.right
            leftMargin: 10
        }
        text: qsTr("Confirm")
        enabled: true
        width: btnWidth
        height: btnHeight + 2
        font: DTK.fontManager.t6

        onClicked: {
            if (saveName === "") {
                exportdialog.visible = false
                emptyWarningDig.show()
                return
            }

            var bRet = albumControl.saveAsImage(filePath , saveName , saveIndex , savefileFormat ,pictureQuality ,saveFolder)
            exportdialog.visible=false
            if (bRet)
                DTK.sendMessage(messageToId, qsTr("Export successful"), "notify_checked")
            else
                DTK.sendMessage(messageToId, qsTr("Export failed"), "warning")

        }
    }

    onVisibleChanged: {
        // 窗口显示时，重置显示内容
        if (visible) {
            nameedit.text = FileControl.slotGetFileName(filePath)
            piczSlider.value = 100
        }

        setX(window.x  + window.width / 2 - width / 2)
        setY(window.y  + window.height / 2 - height / 2)
    }

    function setParameter(path, toId) {
        filePath = path
        messageToId = toId
    }
}
