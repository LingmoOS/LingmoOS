/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.2
import QtGraphicalEffects 1.0
import QtQuick.Window 2.0
import Deepin.Widgets 1.0
import AdjunctUploader 1.0
import DataConverter 1.0
import DataSender 1.0
import "Widgets"

DWindow {
    id:mainWindow

    flags: Qt.FramelessWindowHint
    color: "#00000000"
    width: normalWidth
    height: normalHeight
    shadowWidth: 14
    x: screenSize.width / 2 - width / 2
    y: screenSize.height * 0.15

    property int shadowRadius: 10
    property int normalWidth: 460 + (shadowRadius + rootRec.radius) * 2
    property int normalHeight: 592 + (shadowRadius + rootRec.radius) * 2
    property int maxWidth:screenSize.width <= 1024 ? screenSize.width * 11 /20 : screenSize.width * 9 / 20
    property int maxHeight: screenSize.height <= 768 ? screenSize.height : screenSize.height * 5 / 6
    property string lastTarget: "" //lastTarget = currentTarget if combobox menu item not change
    property int animationDuration: 200
    property bool enableInput: appComboBox.text != "" && appComboBox.labels.indexOf(appComboBox.text) >= 0
    property bool haveDraft: {
        reportTypeButtonRow.reportType != DataConverter.DFeedback_Bug ||
                titleTextinput.text != "" ||
                (emailTextinput.text != "" && emailTextinput.emailChanged) ||
                helpCheck.checked == false ||
                adjunctPanel.haveAdjunct
    }
    property bool draftEdited: false

    property bool sending: false

    property int notifyFailedId: 0

    property bool resendFlag: false

    function updateReportContentText(value){
        adjunctPanel.setContentText(value)
    }

    function updateAdjunctsPathList(list){
        for (var i = 0; i < list.length; i ++){
            adjunctPanel.addAdjunct(list[i])
        }
    }

    function updateSimpleEntries(feedbackType, reportTitle, email, helpDeepin){
        reportTypeButtonRow.reportType = feedbackType

        titleTextinput.text = reportTitle

        emailTextinput.text = email

        helpCheck.checked = helpDeepin
    }

    function saveDraft(){
        if (lastTarget == "" ||!draftEdited )
            return

        mainObject.saveDraft(lastTarget,
                             reportTypeButtonRow.reportType,
                             titleTextinput.text,
                             emailTextinput.text,
                             helpCheck.checked,
                             adjunctPanel.contentText)
    }

    function contentEdited(){
        draftEdited = true

        sendButton.text = dsTr("Send")
    }

    function clearDraft(){
        reportTypeButtonRow.reportType = DataConverter.DFeedback_Bug
        titleTextinput.text = ""
        emailTextinput.text = ""
        emailTextinput.emailChanged = false
        helpCheck.checked = true
        adjunctPanel.clearAllAdjunct()
        //if some attachments is uploading,stop it
        AdjunctUploader.cancelAllUpload()
    }

    function switchProject(project){
        autoSaveTimer.stop()
        saveDraft()

        //project exist, try to load draft
        if (mainObject.draftTargetExist(project)){
            clearDraft()
            //load new target data
            mainObject.updateUiDraftData(project)
            lastTarget = project
        }
        else{
            var emailsList = mainObject.getEmails()
            if (emailsList.length > 0){
                emailTextinput.text = emailsList[0]
            }

            clearDraft()
        }

        appComboBox.setText(project)
        lastTarget = project
        draftEdited = false
        autoSaveTimer.start()
    }

    function isLegitEmail(email){
        var reMail =/^(?:[a-zA-Z0-9]+[_\-\+\.]?)*[a-zA-Z0-9]+@(?:([a-zA-Z0-9]+[_\-]?)*[a-zA-Z0-9]+\.)+([a-zA-Z]{2,})+$/;
        var tmpRegExp = new RegExp(reMail);

        if(tmpRegExp.test(email)){
            return true
        }
        else{
            return false
        }
    }

    function getJsonData(){
        var jsonObj = {
            "method": "Deepin.Feedback.putFeedback",
            "version": "1.1",
            "params": {
                "project" : getBugzillaProjectByName(appComboBox.text),
                "description": adjunctPanel.getDescriptionDetails(),
                "summary" : titleTextinput.text.trim(),
                "attachments": adjunctPanel.getAttachmentsList(),
                "email" : emailTextinput.text.trim(),
                "type" : reportTypeButtonRow.reportType == DataConverter.DFeedback_Bug ? "problem" : "suggestion"
            }
        }

        return marshalJSON(jsonObj)
    }

    function disableTextInput() {
        titleTextinput.enabled = false
        adjunctPanel.enabled = false
        emailTextinput.enabled = false
        helpTextItem.enabled = false
    }

    function disableAllInput(){
        reportTypeButtonRow.enabled = false
        disableTextInput()
    }

    function enableAllInput(){
        reportTypeButtonRow.enabled = true
        titleTextinput.enabled = true
        adjunctPanel.enabled = true
        emailTextinput.enabled = true
        helpTextItem.enabled = true
    }

    function postDataToServer(){
        console.log ("Posting data to bugzilla server...")
        dataSender.postFeedbackData(getJsonData())
        mainObject.saveEmail(emailTextinput.text)
    }

    function resetAllInputState() {
        appComboBox.setTextInputState("normal")
        titleTextinput.state = "normal"
        adjunctPanel.state = "normal"
        emailTextinput.state = "normal"
    }

    Connections {
        target:feedbackContent
        onGenerateReportFinished: {
            var packageList = unmarshalJSON(arg1)
            console.log ("Genera system infomation archive complete! Packages count:", packageList.length)
            adjunctPanel.sysAdjunctCount = packageList.length
            for (var i = 0; i < packageList.length; i ++){
                backendGenerateResult.push(getFileNameByPath(packageList[i]))
                adjunctPanel.getAdjunct(packageList[i])
            }
        }
    }

    DataSender {
        id: dataSender
        onPostFinish: {
            mainObject.clearDraft(lastTarget)
            adjunctPanel.clearAllAdjunct()
            mainWindow.close()
            dataSender.showSuccessNotification(dsTr("Deepin User Feedback")
                                               ,dsTr("Your feedback has been sent successfully, thanks for your support!"))

            Qt.quit()
        }
        onPostError: {
            console.warn ("Post data error:", message)
            sendButton.text = dsTr("Resend")
            sending = false
            closeButton.enabled = true
            resendFlag = true
            windowButtonRow.closeEnable = true

            //enable all UI
            enableAllInput()
            appComboBox.enabled = true

            mainObject.clearSysAdjuncts(lastTarget)

            notifyFailedId = dataSender.showErrorNotification(
                dsTr("Deepin User Feedback"),
                dsTr("Failed to send your feedback, resend?"),
                dsTr("Resend"))

        }
        onRetryPost: {
            if (sendButton.enabled){
                sendButton.text = dsTr("Sending ...")
                sending = true
                closeButton.enabled = false
                windowButtonRow.closeEnable = false

                //disable UI
                disableAllInput()
                appComboBox.enabled = false

                backendGenerateResult = []
                //genera system infomation,then send data to server
                feedbackContent.GenerateReport(getProjectIDByName(appComboBox.text), helpCheck.checked)
            }
        }
    }

    Connections {
        target: AdjunctUploader

        onUploadFailed:{
            // BackendGenerateResult will only upload when "Send" button click
            //so ignore other error here
            if (isBackendGenerateResult(filePath)) {
                console.warn ("Post data error:", message)
                sendButton.text = dsTr("Resend")
                sending = false
                closeButton.enabled = true
                resendFlag = true
                windowButtonRow.closeEnable = true

                //enable all UI
                enableAllInput()
                appComboBox.enabled = true

                mainObject.clearSysAdjuncts(lastTarget)

                dataSender.showErrorNotification(dsTr("Deepin User Feedback")
                                                 ,dsTr("Failed to send your feedback, resend?")
                                                 ,dsTr("Resend"))
            }
        }
    }

    Timer {
        id: autoSaveTimer
        running: true
        repeat: true
        interval: 2000
        onTriggered: saveDraft()
    }

    Rectangle {
        id: rootRec
        anchors.centerIn: parent

        width: mainWindow.width - (shadowRadius + rootRec.radius) * 2
        height: mainWindow.height - (shadowRadius + rootRec.radius) * 2
        radius: 4
        border.width: 1
        border.color: Qt.rgba(0, 0, 0, 0.2)

        DDragableArea {
            anchors.fill: parent
            window: mainWindow
            dragStartX: 0
            dragStartY: 0
            windowLastX: mainWindow.width
            windowLastY: mainWindow.height
        }

        DropArea {
            id: mainDropArea
            enabled: enableInput
            anchors.fill: parent
            width: parent.width
            height: parent.height
            onDropped: {
                adjunctPanel.hideAddAdjunctIcon()
                adjunctPanel.warning = false

                if (!adjunctPanel.canAddAdjunct)
                    return

                for (var key in drop.urls){
                    if (adjunctPanel.canAddAdjunct){
                        adjunctPanel.getAdjunct(drop.urls[key].slice(7,drop.urls[key].length))
                    }
                }
            }
            onEntered: {
                if (adjunctPanel.canAddAdjunct)
                    adjunctPanel.showAddAdjunctIcon(drag.urls.length)
                else{
                    adjunctPanel.warning = true
                    if (enableInput)
                        toolTip.showTip(dsTr("Total attachments have reached limit. "))
                }
            }
            onExited: {
                adjunctPanel.hideAddAdjunctIcon()
                adjunctPanel.warning = false
                toolTip.hideTip()
            }
        }

        TitleRow {
            width: parent.width - windowButtonRow.width
            height: 16
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 8
        }

        WindowButtonRow {
            id:windowButtonRow
            anchors.top:parent.top
            anchors.right: parent.right
        }

        ReportTypeButtonRow {
            id: reportTypeButtonRow
            width: rootRec.width - 22 * 2
            anchors.top: rootRec.top
            anchors.topMargin: 38
            anchors.horizontalCenter: parent.horizontalCenter
        }

        AppComboBox {
            id:appComboBox
            parentWindow: mainWindow
            height: 30
            width: reportTypeButtonRow.width
            anchors.top: reportTypeButtonRow.bottom
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            onMenuSelect: {
                if (lastTarget != "" && haveDraft && draftEdited){
                    toolTip.showTipWithColor(dsTr("The draft of %1 has been saved.").arg(getProjectNameByID(lastTarget)),"#a4a4a4")
                }
                switchProject(projectList[index])
            }
            onSearchResultCountChanged: {
                if (count > 0)
                    enableAllInput()
                else
                    disableAllInput()
            }
        }

        AppTextInput {
            id: titleTextinput
            state: "normal"
            enabled: enableInput
            backgroundColor: enabled ? bgNormalColor : inputDisableBgColor
            width: reportTypeButtonRow.width
            height: 30
            maxStrLength: 100
            anchors.top: appComboBox.bottom
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            tip:reportTypeButtonRow.reportType == DataConverter.DFeedback_Bug ? dsTr("Please input the problem title")
                                                            : dsTr("Please describe your idea simply")
            onInWarningStateChanged: {
                if (inWarningState){
                    toolTip.showTip(dsTr("Title words have reached limit."))
                }
            }
            onTextChange: contentEdited()

        }

        AdjunctPanel {
            id:adjunctPanel

            enabled: enableInput
            backgroundColor: enabled ? bgNormalColor : inputDisableBgColor
            width: reportTypeButtonRow.width
            height: (rootRec.height - 340)
            anchors.top: titleTextinput.bottom
            anchors.topMargin: 6
            anchors.horizontalCenter: parent.horizontalCenter
            onSysAdjunctUploaded: {
                postDataToServer()
            }
        }

        AppTextInput {
            id: emailTextinput
            state: "normal"
            property bool canFillEmail: true
            property bool emailChanged: false
            enabled: enableInput
            backgroundColor: enabled ? bgNormalColor : inputDisableBgColor
            width: reportTypeButtonRow.width
            height: 30
            anchors.top: adjunctPanel.bottom
            anchors.topMargin: 18
            anchors.horizontalCenter: parent.horizontalCenter
            tip: dsTr("Please fill in email to get the feedback progress.")
            onFocusChanged: {
                if (!focus && !isLegitEmail(emailTextinput.text)){
                    toolTip.showTip(dsTr("Email is invalid."))
                }
            }
            onKeyPressed: {
                if (event.key == Qt.Key_Backspace || event.key == Qt.Key_Delete){
                    canFillEmail = false
                }
                else if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return || event.key == Qt.Key_Right){
                    canFillEmail = false
                    input.deselect()
                }
                else{
                    canFillEmail = true
                }
            }
            onTextChange: {
                emailChanged = true
                contentEdited()
                if (canFillEmail){
                    var startIndex = input.cursorPosition
                    if (startIndex >= input.text.length && input.text.length > 0){
                        var matchEmail = mainObject.getMatchEmailPart(text)
                        input.text = text + matchEmail
                        input.select(startIndex, input.text.length)
                    }
                }
                if (input.selectionStart == 0){//change by combobox
                    input.deselect()
                    emailChanged = false
                }
            }
        }

        Item {
            id: helpTextItem
            anchors.top: emailTextinput.bottom
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            width: reportTypeButtonRow.width
            height: helpText.contentHeigh

            AppCheckBox {
                id: helpCheck
                enabled: enableInput && parent.enabled
                width: 15
                anchors.left: parent.left
                anchors.top: parent.top
                checked: true
                onCheckedChanged: contentEdited()
            }

            Text {
                id: helpText
                enabled: enableInput && parent.enabled
                anchors.left: helpCheck.right
                lineHeight: 1.6
                width: parent.width - helpCheck.width
                text: dsTr("I wish to join in User Feedback Help Plan to quickly improve the system without any personal information collected.")
                wrapMode: Text.Wrap
                color: enabled ? textNormalColor : "#bebebe"
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: 12
                clip: true
            }
        }

        Row {
            id: controlButtonRow
            anchors.right: reportTypeButtonRow.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 16
            spacing: 8

            TextButton {
                id:closeButton
                text: dsTr("Close")
                textItem.color: enabled ? textNormalColor : "#bebebe"
                onClicked: {
                    saveDraft()
                    mainWindow.close()
                    Qt.quit()
                }
            }

            TextButton {
                id: sendButton
                width: textItem.contentWidth < 40 ? 60 : textItem.contentWidth + 50
                text: dsTr("Send")
                enabled: {
                    if (titleTextinput.text != "" && !titleTextinput.inWarningState
                            && appComboBox.text != ""
                            && adjunctPanel.getDescriptionDetails() !== ""
                            && adjunctPanel.isAllAttachmentsUploaded
                            && isLegitEmail(emailTextinput.text)
                            && (!mainWindow.sending)){
                        textItem.color = textNormalColor
                        return true
                    }else{
                        textItem.color = "#bebebe"
                        return false
                    }
                }
                onClicked: {
                    //for deepin-feedback-web,not collect sys info
                    if (getProjectIDByName(appComboBox.text) == "none"){
                        postDataToServer()
                        return
                    }

                    text = dsTr("Sending ...")
                    sending = true
                    if (resendFlag) {
                        resendFlag = false
                        dataSender.closeNotification(notifyFailedId)
                    }
                    closeButton.enabled = false
                    windowButtonRow.closeEnable = false

                    // reset text input state
                    resetAllInputState()

                    //disable UI
                    disableAllInput()
                    appComboBox.enabled = false

                    console.log ("Generating system infomation archive...")
                    backendGenerateResult = []
                    //genera system infomation,then send data to server
                    feedbackContent.GenerateReport(getProjectIDByName(appComboBox.text), helpCheck.checked)
                }
            }
        }

        Tooltip {
            id: toolTip
            anchors.left: adjunctPanel.left
            anchors.verticalCenter: controlButtonRow.verticalCenter
            autoHideInterval: 3600
            height: textItem.lineCount == 1 ? textItem.contentHeight : controlButtonRow.height
            maxWidth: parent.width - controlButtonRow.width - 50
        }
    }

    RectangularGlow {
        id: shadow
        z: -1
        anchors.fill: rootRec
        glowRadius: shadowRadius
        spread: 0.2
        color: Qt.rgba(0, 0, 0, 0.3)
        cornerRadius: rootRec.radius + shadowRadius
        visible: true
    }
}
