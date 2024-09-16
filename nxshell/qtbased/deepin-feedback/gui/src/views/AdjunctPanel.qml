/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.1
import Deepin.Widgets 1.0
import QtQuick.Dialogs 1.1
import DataConverter 1.0
import "Widgets"


FocusScope{
    id: adjunctPanel
    state: "normal"

    property alias backgroundColor: adjunctRec.color
    property alias contentText: contentTextEdit.text
    property bool isAllAttachmentsUploaded: adjunctTray.available
    property bool canAddAdjunct: adjunctTray.adjunctModel.count < maxAdjunctCount && enableInput
    property bool showAddAdjunct: false
    property int addingCount: 0
    property bool warning: false
    property real singleMaxSize: 15 * 1024 * 1024
    property bool haveAdjunct: contentTextEdit.text != "" || adjunctTray.adjunctModel.count > 0
    property int sysAdjunctCount: 0

    signal sysAdjunctUploaded()

    function clearAllAdjunct(){
        contentTextEdit.text = ""
        adjunctTray.clearAllAdjunct()
    }

    function getAdjunct(filePath){
        if (mainObject.adjunctExist(filePath,getProjectIDByName(appComboBox.text.trim()))){
            console.log ("Attachment already exist: ", filePath)
            return
        }

        if (mainObject.getAdjunctSize(filePath) > singleMaxSize){
            toolTip.showTip(dsTr("Failed to add attachment: Single attachment can not be more than 15M."))
            return
        }

        var targetPath = mainObject.addAdjunct(filePath, getProjectIDByName(appComboBox.text.trim()))
        if (targetPath == ""){
            console.log ("Adjunct already exist or got attachment error")
            return
        }

        adjunctTray.addAdjunct(targetPath)
    }

    function addAdjunct(filePath){
        adjunctTray.addAdjunct(filePath)
    }

    function setContentText(value){
        contentTextEdit.text = value
    }

    function updateLoadPercent(filePath, percent){
        adjunctTray.updateLoadPercent(filePath, percent)
    }

    function showAddAdjunctIcon(count){
        showAddAdjunct = true
        adjunctTray.showAddIcon(count)
        addingCount = count
    }

    function hideAddAdjunctIcon(){
        showAddAdjunct = false
        adjunctTray.hideAddIcon(addingCount)
        addingCount = 0
    }

    function getDescriptionDetails(){
        return contentTextEdit.text
    }

    function getAttachmentsList(){
        return adjunctTray.getAdjunctList()
    }

//    function isAllAttachmentsUploaded(){
//        return adjunctTray.available
//    }

    onWarningChanged: {
        if (warning)
            state = "warning"
        else{
            if (focus)
                state = "actived"
            else
                state = "normal"
        }
    }

    onFocusChanged: {
        if (!warning){
            if (focus)
                state = "actived"
            else
                state = "normal"
        }
    }

    states: [
        State {
            name: "normal"
            PropertyChanges {target: adjunctRec; border.color: buttonBorderColor}
        },
        State {
            name: "actived"
            PropertyChanges {target: adjunctRec; border.color: buttonBorderActiveColor}
        },
        State {
            name: "warning"
            PropertyChanges {target: adjunctRec; border.color: buttonBorderWarningColor}
        }
    ]

    Item {
        width: parent.width
        height: parent.height

        //use Component.createObject() to makesure the files propertys will be refresh
        //there is a bug with FileDialog
        Component {
            id: pickerDialogComp

            FileDialog {
                title: dsTr("Please select attachment")
                onAccepted: {
                    adjunctPanel.getAdjunct(fileUrl.toString().replace("file:///","/"))
                    close()
                }
                nameFilters: [ "All files (*)" ,"Image files (*.jpg *.png *.gif)"]
            }
        }

        Row {
            id: buttonRow
            anchors.left: parent.left
            anchors.top: parent.top
            width: childrenRect.width
            height: 22

            spacing: 3

            DImageButton {
                id:screenShotButton
                width: 22
                height: 22
                normal_image: "qrc:/views/Widgets/images/screenshot_%1.png".arg(canAddAdjunct ? "normal" : "disable")
                hover_image: "qrc:/views/Widgets/images/screenshot_%1.png".arg(canAddAdjunct ? "press" : "disable")
                press_image: "qrc:/views/Widgets/images/screenshot_%1.png".arg(canAddAdjunct ? "press" : "disable")
                onClicked: {
                    if (canAddAdjunct
                            && appComboBox.text != ""
                            && projectNameList.indexOf(appComboBox.text) != -1){

                        mainWindow.hide()
                        mainObject.getScreenShot(appComboBox.text.trim())
                    }
                }
                onEntered: {
                    if (!canAddAdjunct)
                        toolTip.showTip(dsTr("Total attachments have reached limit. "))
                }
            }

            DImageButton {
                id:adjunctButton
                width: 22
                height: 22
                normal_image: "qrc:/views/Widgets/images/adjunct_%1.png".arg(canAddAdjunct ? "normal" : "disable")
                hover_image: "qrc:/views/Widgets/images/adjunct_%1.png".arg(canAddAdjunct ? "press" : "disable")
                press_image: "qrc:/views/Widgets/images/adjunct_%1.png".arg(canAddAdjunct ? "press" : "disable")
                onClicked: {
                    if (canAddAdjunct
                            && appComboBox.text != ""
                            && projectNameList.indexOf(appComboBox.text) != -1){

                        pickerDialogComp.createObject(adjunctPanel).open()
                    }
                }
                onEntered: {
                    if (!canAddAdjunct)
                        toolTip.showTip(dsTr("Total attachments have reached limit. "))
                }
            }
        }

        Rectangle {
            id: adjunctRec

            radius: 3
            color: bgNormalColor
            width: parent.width
            height: parent.height - buttonRow.height
            anchors.top: buttonRow.bottom
            anchors.topMargin: 2

            ListView {
                id: textEditView
                width: parent.width
                height: parent.height - adjunctTray.height
                anchors.top: parent.top
                anchors.topMargin:0
                anchors.horizontalCenter: parent.horizontalCenter
                model: itemModel
                clip: true

//                DScrollBar {
//                    flickable: textEditView
//                }
            }

            VisualItemModel
            {
                id: itemModel

                Item {
                    width: adjunctRec.width
                    height: contentTextEdit.contentHeight > textEditView.height ? contentTextEdit.contentHeight : textEditView.height

                    Flickable {
                        id: flick

                        width: textEditView.width;
                        height: textEditView.height;
                        contentWidth: contentTextEdit.paintedWidth
                        contentHeight: contentTextEdit.paintedHeight
                        clip: true

                        function ensureVisible(r)
                        {
                            if (contentX >= r.x)
                                contentX = r.x;
                            else if (contentX+width <= r.x+r.width)
                                contentX = r.x+r.width-width;
                            if (contentY >= r.y)
                                contentY = r.y;
                            else if (contentY+height <= r.y+r.height)
                                contentY = r.y+r.height-height;
                        }

                        TextEdit {
                            id: contentTextEdit
                            width: flick.width
                            height: flick.height
                            textMargin: 6
                            focus: true
                            color: adjunctPanel.enabled ? textNormalColor : "#bebebe"
                            selectionColor: "#61B5F8"
                            selectByMouse: true
                            font.pixelSize: 12
                            wrapMode: TextEdit.Wrap

                            onTextChanged: contentEdited()
                            onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
                        }
                    }

                    TextEdit {
                        id: problemTips
                        textMargin: 6
                        color: "#bebebe"
                        font.pixelSize: 12
                        enabled: false
                        width: contentTextEdit.width
                        height: contentTextEdit.height
                        wrapMode: TextEdit.Wrap
                        opacity: contentTextEdit.text == ""
                                 && reportTypeButtonRow.reportType == DataConverter.DFeedback_Bug
                                 && adjunctPanel.state == "normal"
                                 ? 1 : 0
                        text: {
                            var content = dsTr("Please describe your problem in detail") + "\n\n" +
                                    dsTr("Please upload related screenshots or files") + "\n\n" +
                                    dsTr("Single attachment can not exceed 15M") + "\n\n" +
                                    dsTr("The total number of attachments is not more than six")
                            return content
                        }

                        Behavior on opacity {
                            NumberAnimation {duration: 150}
                        }
                    }

                    TextEdit {
                        id: ideaTips
                        textMargin: 6
                        color: "#bebebe"
                        font.pixelSize: 12
                        enabled: false
                        width: contentTextEdit.width
                        height: contentTextEdit.height
                        wrapMode: TextEdit.Wrap
                        opacity: contentTextEdit.text == ""
                                 && reportTypeButtonRow.reportType != DataConverter.DFeedback_Bug
                                 && adjunctPanel.state == "normal"
                                 ? 1 : 0
                        text: {
                            var content = dsTr("Please describe your idea in detail") + "\n\n" +
                                    dsTr("Please upload related attachments") + "\n\n" +
                                    dsTr("Single attachment can not exceed 15M") + "\n\n" +
                                    dsTr("The total number of attachments is not more than six")
                            return content
                        }

                        Behavior on opacity {
                            NumberAnimation {duration: 150}
                        }
                    }
                }
            }


            AdjunctTray {
                id: adjunctTray
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 1
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2
                height: (adjunctModel.count !=0 || showAddAdjunct) ? 52 : 0
                Behavior on height {
                    NumberAnimation {duration: 150}
                }
                onAdjunctAdded: contentEdited()
                onAdjunctRemoved: contentEdited()
                onSysAdjunctUploaded: {
                    adjunctPanel.sysAdjunctUploaded()
                }

            }
        }

        Connections {
            target: mainObject
            onGetScreenshotFinish: {
                getAdjunct(fileName)
                mainWindow.show()
            }
        }
    }
}


