
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0
import org.lingmo.dtk.style 1.0 as DS

OceanUIObject {
    id: settings
    property string userId
    property string papaName
    property bool autoLoginChecked: true
    property bool nopasswdLoginChecked: true

    Component.onCompleted: {
        settings.autoLoginChecked = oceanuiData.autoLogin(settings.userId)
        settings.nopasswdLoginChecked = oceanuiData.nopasswdLogin(settings.userId)
    }

    // 账户头像
    OceanUIObject {
        id: userAvatars
        name: "userAvatars"
        parentName: settings.papaName
        weight: 10
        pageType: OceanUIObject.Item
        page: Rectangle {
            id: item
            color: "transparent"
            implicitHeight: 100

            Control {
                id: control
                implicitWidth: 100
                implicitHeight: 100
                antialiasing: true
                hoverEnabled: true
                MouseArea {
                    anchors.fill: parent
                    Loader {
                        id: dilogLoader
                        active: false
                        sourceComponent: AvatarSettingsDialog {
                            id: dialog
                            userId: settings.userId
                            currentAvatar: oceanuiData.avatar(settings.userId)
                            onAccepted: {
                                if (currentAvatar.length > 0)
                                    oceanuiData.setAvatar(settings.userId, currentAvatar)
                            }

                            onClosing: function (close) {
                                dilogLoader.active = false
                            }
                        }
                        onLoaded: function () {
                            dilogLoader.item.show()
                        }
                    }

                    Connections {
                        target: oceanuiData
                        function onAvatarChanged(userId, avatar) {
                            if (userId === settings.userId) {
                                image.source = oceanuiData.avatar(userId)
                            }
                        }
                        function onAutoLoginChanged(userId, enable) {
                            if (userId === settings.userId) {
                                settings.autoLoginChecked = enable
                            }
                        }
                        function onNopasswdLoginChanged(userId, enable) {
                            if (userId === settings.userId) {
                                settings.nopasswdLoginChecked = enable
                            }
                        }
                        function onUserRemoved(userId) {
                            if (userId === settings.userId) {
                                OceanUIApp.showPage("accounts")
                            }
                        }
                    }

                    onClicked: {
                        dilogLoader.active = true
                    }
                }

                // crashed https://github.com/lingmoos/dtkdeclarative/pull/385
                // ToolTip.text: qsTr("Clicked to changed avatar")
                // ToolTip.visible: control.hovered
                Image {
                    id: image
                    source: oceanuiData.avatar(settings.userId)
                    asynchronous: true
                    anchors.fill: parent
                    width: 100
                    height: 100
                    fillMode: Image.PreserveAspectCrop
                    clip: true
                }

                Rectangle {
                    width: 100
                    height: 30
                    color: Qt.rgba(0, 0, 0, 0.5)
                    visible: control.hovered
                    anchors {
                        bottom: image.bottom
                        horizontalCenter: image.horizontalCenter
                    }
                }

                Text {
                    id: editText
                    text: qsTr("edit")
                    color: "white"
                    anchors {
                        horizontalCenter: control.horizontalCenter
                        bottom: control.bottom
                        bottomMargin: 2
                    }
                }

                // fake round image, just set shadowColor same with window color
                BoxShadow {
                    id: boxShadow
                    hollow: true
                    anchors.fill: image
                    shadowBlur: 10
                    spread: 20
                    shadowColor: control.palette.window
                    cornerRadius: control.width / 2
                }
            }

            ColumnLayout {
                id: columLayout
                anchors.left: control.right
                anchors.leftMargin: 10
                anchors.verticalCenter: item.verticalCenter
                Text {
                    text: oceanuiData.userName(settings.userId)
                    font.pointSize: 16
                    font.bold: true
                    color: palette.text // not update ?
                }
                Text {
                    id: userTypeName
                    text: oceanuiData.userTypeName(settings.userId)
                    color: palette.text

                    Connections {
                        target: oceanuiData
                        function onUserTypeChanged(userId, type) {
                            if (userId === settings.userId) {
                                userTypeName.text = oceanuiData.userTypeName(settings.userId)
                            }
                        }
                    }
                }
            }
            RowLayout {
                anchors {
                    left: columLayout.right
                    right: parent.right
                    leftMargin: 10
                    verticalCenter: item.verticalCenter
                }
                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Add new user")
                    Layout.alignment: Qt.AlignRight | Qt.AlignHCenter
                    Layout.rightMargin: 10
                    onClicked: {
                        cadLoader.active = true
                    }
                    Loader {
                        id: cadLoader
                        active: false
                        sourceComponent: CreateAccountDialog {
                            onClosing: function (close) {
                                cadLoader.active = false
                            }
                        }
                        onLoaded: function () {
                            cadLoader.item.show()
                        }
                    }
                }
            }
        }
    }

    // 账户信息
    OceanUIObject {
        name: "acountInfosTitle"
        parentName: settings.papaName
        displayName: qsTr("Acount information")
        weight: 18
        pageType: OceanUIObject.Item
        page: Label {
            leftPadding: 5
            text: oceanuiObj.displayName
            font {
                pointSize: 13
                bold: true
            }
        }
        onParentItemChanged: item => { if (item) item.topPadding = 10 }
    }
    OceanUIObject {
        name: settings.papaName + "acountInfos"
        parentName: settings.papaName
        displayName: qsTr("Acount Information")
        description: qsTr("Acount name, acount fullname, account type")
        weight: 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: settings.papaName + "acountName"
            parentName: settings.papaName + "acountInfos"
            displayName: qsTr("Acount name")
            weight: 10
            pageType: OceanUIObject.Editor
            page: Label {
                text: oceanuiData.userName(settings.userId)
            }
        }
        OceanUIObject {
            name: settings.papaName + "acountFullname"
            parentName: settings.papaName + "acountInfos"
            displayName: qsTr("Acount fullname")
            weight: 20
            pageType: OceanUIObject.Editor
            page: RowLayout {
                EditActionLabel {
                    id: fullNameEdit
                    implicitWidth: 200
                    text: oceanuiData.fullName(settings.userId)
                    placeholderText: qsTr("Set fullname")
                    horizontalAlignment: TextInput.AlignRight
                    editBtn.visible: readOnly
                    onTextEdited: {
                        if (showAlert)
                            showAlert = false
                        // validtor can not paste invalid text..
                        var regex = /^[^:]{0,32}$/
                        if (!regex.test(text)) {
                            var filteredText = text
                            filteredText = filteredText.replace(":", "")

                            // 长度 32
                            filteredText = filteredText.slice(0, 32)
                            text = filteredText
                        }
                    }

                    onFinished: function () {
                        let alertMsg = oceanuiData.checkFullname(text)
                        if (alertMsg.length > 0) {
                            showAlert = false
                            showAlert = true
                            alertText = alertMsg
                            readOnly = false
                            return
                        }

                        if (text.trim().length === 0) {
                            text = oceanuiData.fullName(settings.userId)
                            return
                        }

                        oceanuiData.setFullname(settings.userId, text)
                    }
                    Connections {
                        target: oceanuiData
                        function onFullnameChanged(userId, fullname) {
                            if (userId === settings.userId) {
                                fullNameEdit.text = oceanuiData.fullName(settings.userId)
                            }
                        }
                    }
                }
            }
        }
        OceanUIObject {
            name: settings.papaName + "acountType"
            parentName: settings.papaName + "acountInfos"
            displayName: qsTr("Acount type")
            weight: 30
            pageType: OceanUIObject.Editor
            page: ComboBox {
                flat: true
                model: oceanuiData.userTypes()
                currentIndex: oceanuiData.userType(settings.userId)
                enabled: oceanuiData.isDeleteAble(settings.userId)
                onActivated: function (index) {
                    oceanuiData.setUserType(settings.userId, index)
                }
            }
        }
    }

    // 登陆设置
    OceanUIObject {
        name: settings.papaName + "acountSettingsTitle"
        parentName: settings.papaName
        displayName: qsTr("Login settings")
        weight: 28
        pageType: OceanUIObject.Item
        visible: acountSettings.visible
        page: Label {
            leftPadding: 5
            text: oceanuiObj.displayName
            font {
                pointSize: 13
                bold: true
            }
        }
        onParentItemChanged: item => { if (item) item.topPadding = 10 }
    }
    OceanUIObject {
        id: acountSettings
        name: settings.papaName + "acountSettings"
        parentName: settings.papaName
        displayName: qsTr("Login Settings")
        description: qsTr("Auto longin, login without password")
        weight: 30
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        visible: autoLongin.visible || noPassword.visible

        OceanUIObject {
            id: autoLongin
            name: settings.papaName + "autoLongin"
            parentName: settings.papaName + "acountSettings"
            displayName: qsTr("Auto longin")
            weight: 10
            pageType: OceanUIObject.Editor
            visible: oceanuiData.isAutoLoginVisable()
            enabled: oceanuiData.currentUserId() === settings.userId
            page: Switch {
                checked: settings.autoLoginChecked
                onCheckedChanged: {
                    if (settings.autoLoginChecked != checked)
                        settings.autoLoginChecked = checked

                    oceanuiData.setAutoLogin(settings.userId, checked)
                }
            }
        }

        OceanUIObject {
            id: noPassword
            name: settings.papaName + "noPassword"
            parentName: settings.papaName + "acountSettings"
            displayName: qsTr("Login without password")
            weight: 20
            pageType: OceanUIObject.Editor
            visible: oceanuiData.isNoPassWordLoginVisable()
            enabled: oceanuiData.currentUserId() === settings.userId
            page: Switch {
                checked: settings.nopasswdLoginChecked
                onCheckedChanged: {
                    if (settings.nopasswdLoginChecked != checked)
                        settings.nopasswdLoginChecked = checked

                    oceanuiData.setNopasswdLogin(settings.userId, checked)
                }
            }
        }
    }

    // 登陆方式
    LoginMethod {
        name: settings.papaName + "loginMethodTitle"
        parentName: settings.papaName
        userId: settings.userId
    }

    // 动态锁
    // OceanUIObject {
    //     name: settings.papaName + "dynamicLockTitle"
    //     parentName: settings.papaName
    //     displayName: qsTr("Dynamic lock screen")
    //     weight: 48
    //     pageType: OceanUIObject.Item
    //     page: Label {
    //         leftPadding: 5
    //         text: oceanuiObj.displayName
    //         font {
    //             pointSize: 13
    //             bold: true
    //         }
    //     }
    //     onParentItemChanged: item => { if (item) item.topPadding = 10 }
    // }
    // OceanUIObject {
    //     name: settings.papaName + "dynamicLockGroups"
    //     parentName: settings.papaName
    //     displayName: qsTr("Dynamic lock screen groups")
    //     weight: 50
    //     pageType: OceanUIObject.Item
    //     page: OceanUIGroupView {}

    //     OceanUIObject {
    //         name: settings.papaName + "dynamicLockItem"
    //         parentName: settings.papaName + "dynamicLockGroups"
    //         displayName: qsTr("Lock screen when device is disconnected")
    //         description: qsTr("Lock screen when the bound Bluetooth device is disconnected")
    //         weight: 10
    //         pageType: OceanUIObject.Editor
    //         page: Switch {}
    //     }
    // }
    OceanUIObject {
        id: bottomButtons
        name: settings.papaName + "/bottomButtons"
        parentName: settings.papaName
        weight: 0xFFFF
        pageType: OceanUIObject.Item
        page: RowLayout {
            Button {
                Layout.alignment: groupSettingsBtn.visible ? Qt.AlignLeft : Qt.AlignRight
                text: qsTr("Delete current account")
                enabled: oceanuiData.isDeleteAble(settings.userId)
                Loader {
                    id: cfdLoader
                    active: false
                    sourceComponent: ComfirmDeleteDialog {
                        onClosing: function (close) {
                            cfdLoader.active = false
                        }
                        onRequestDelete: function (deleteHome) {
                            oceanuiData.removeUser(settings.userId, deleteHome)
                        }
                    }
                    onLoaded: function () {
                        cfdLoader.item.show()
                    }
                }
                onClicked: {
                    cfdLoader.active = true
                }
            }

            Button {
                id: groupSettingsBtn
                Layout.alignment: Qt.AlignRight
                text: qsTr("Group setting")
                visible: oceanuiData.needShowGroups()
                onClicked: {
                    OceanUIApp.showPage(groupSettings)
                }
            }
        }
    }

    OceanUIObject {
        id: groupSettings
        property bool isEditing
        property int lrMargin: 60
        name: settings.papaName + "/groupSettings"
        parentName: bottomButtons.name
        displayName: qsTr("Account groups")
        weight: 10
        pageType: OceanUIObject.Menu
        page: ListView {
            id: groupview
            spacing: 1
            anchors {
                left: parent ? parent.left : undefined
                right: parent ? parent.right : undefined
            }

            ScrollBar.vertical: ScrollBar {
                width: 10
            }

            header: Item {
                implicitHeight: 50
                anchors {
                    left: parent ? parent.left : undefined
                    right: parent ? parent.right : undefined
                    leftMargin: groupSettings.lrMargin
                    rightMargin: groupSettings.lrMargin
                }
                RowLayout {
                    anchors.fill: parent
                    Label {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        Layout.leftMargin: 10
                        font.bold: true
                        font.pointSize: 14
                        text: oceanuiObj.displayName
                    }

                    Button {
                        id: button
                        checkable: true
                        checked: groupSettings.isEditing
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        Layout.rightMargin: 10
                        text: groupSettings.isEditing ? qsTr("done") : qsTr("edit")
                        font.pointSize: 13
                        background: null
                        textColor: Palette {
                            normal {
                                common: DTK.makeColor(Color.Highlight)
                                crystal: DTK.makeColor(Color.Highlight)
                            }
                        }
                        onCheckedChanged: {
                            groupSettings.isEditing = button.checked
                        }
                    }
                }
            }

            model: settings.userId.length > 0 ? oceanuiData.groupsModel(settings.userId) : 0
            delegate: ItemDelegate {
                implicitHeight: 50
                checkable: false
                enabled: model.groupEnabled
                background: OceanUIItemBackground {
                    backgroundType: OceanUIObject.Normal
                    separatorVisible: true
                }
                anchors {
                    left: parent ? parent.left : undefined
                    right: parent ? parent.right : undefined
                    leftMargin: groupSettings.lrMargin
                    rightMargin: groupSettings.lrMargin
                }

                contentItem: RowLayout {
                    EditActionLabel {
                        id: editLabel
                        property bool editAble: model.groupEditAble
                        text: model.display
                        validator: RegularExpressionValidator {
                            // 仅使用字母、数字、下划线或短横线，并且以字母开头
                            regularExpression: /[a-zA-Z][a-zA-Z0-9-_]{0,31}$/
                        }
                        implicitHeight: 40
                        implicitWidth: 200
                        placeholderText: qsTr("Group name")
                        horizontalAlignment: TextInput.AlignLeft | Qt.AlignVCenter
                        editBtn.visible: readOnly && editAble
                                         && !groupSettings.isEditing
                        readOnly: model.display.length > 0

                        onFinished: function () {
                            if (text.length < 1) {
                                text = model.display
                                return
                            }
                            if (model.display.length < 1)
                                oceanuiData.createGroup(text)
                            else
                                oceanuiData.modifyGroup(model.display, text)
                        }
                        onFocusChanged: {
                            if (focus || text.length > 0 || editLabel.readonly)
                                return

                            if (model.display.length < 1) {
                                oceanuiData.requestClearEmptyGroup(settings.userId)
                                return
                            }

                            text = model.display
                        }
                        Component.onCompleted: {
                            if (editLabel.readOnly)
                                return

                            Qt.callLater(function () {
                                editLabel.focus = true
                            })
                        }
                    }

                    ActionButton {
                        id: editButton
                        focusPolicy: Qt.NoFocus
                        icon.width: 18
                        icon.height: 18
                        visible: groupSettings.isEditing ? editLabel.editAble : true
                        icon.name: {
                            if (groupSettings.isEditing) {
                                return "list_delete"
                            }

                            return editButton.checked ? "item_checked" : "item_unchecked"
                        }

                        background: null
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        // list_delete.dci icon has padding 10 (1.10p)
                        Layout.rightMargin: groupSettings.isEditing ? 0 : 10
                        // only checked from groupsChanged
                        checkable: false
                        checked: oceanuiData.groupContains(settings.userId, model.display)
                        onClicked: {
                            if (groupSettings.isEditing) {
                                // delete group
                                oceanuiData.deleteGroup(model.display)
                                groupSettings.isEditing = false
                                return
                            }

                            oceanuiData.setGroup(settings.userId, model.display, !editButton.checked)
                        }
                    }
                }

                corners: getCornersForBackground(index, groupview.count)
            }

            footer: Item {
                implicitHeight: 50
                anchors {
                    left: parent ? parent.left : undefined
                    right: parent ? parent.right : undefined
                    leftMargin: groupSettings.lrMargin
                    rightMargin: groupSettings.lrMargin
                }
                RowLayout {
                    anchors.fill: parent
                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Add group")
                        onClicked: {
                            oceanuiData.requestCreateGroup(settings.userId)
                            groupview.positionViewAtEnd()
                        }
                    }
                }
            }
        }
    }
}
