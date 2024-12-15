// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    property var appList: oceanuiData.appItemModels
    OceanUIObject {
        name: "doNotDisturbNotification"
        parentName: "notification"
        displayName: qsTr("Do Not Disturb Settings")
        weight: 10
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Label {
                height: contentHeight
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
        }
    }
    OceanUIObject {
        name: "enableDoNotDisturb"
        parentName: "notification"
        // displayName: qsTr("Enable Do Not Disturb")
        // icon: "notification"
        weight: 20
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "enableDoNotDisturbSwitch"
            parentName: "enableDoNotDisturb"
            description: qsTr("App notifications will not be shown on desktop and the sounds will be silenced, but you can view all messages in the notification center.")
            displayName: qsTr("Enable Do Not Disturb")
            // icon: "notification"
            weight: 10
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: oceanuiData.sysItemModel.disturbMode
                onCheckedChanged: {
                    if (oceanuiData.sysItemModel.disturbMode !== checked) {
                        oceanuiData.sysItemModel.disturbMode = checked
                    }
                }
            }
        }
        OceanUIObject {
            name: "enableDoNotDisturbTime"
            parentName: "enableDoNotDisturb"
            displayName: qsTr("Enable Do Not Disturb")
            icon: "notification"
            weight: 20
            pageType: OceanUIObject.Item
            visible: oceanuiData.sysItemModel.disturbMode
            page: TimeRange {
            }
        }
        OceanUIObject {
            name: "enableDoNotDisturbLock"
            parentName: "enableDoNotDisturb"
            displayName: qsTr("Enable Do Not Disturb")
            icon: "notification"
            weight: 30
            pageType: OceanUIObject.Item
            visible: oceanuiData.sysItemModel.disturbMode
            page: RowLayout {
                D.CheckBox {
                    id: lockScreenCheckBox
                    implicitHeight: implicitContentHeight + 30
                    checked: oceanuiData.sysItemModel.lockScreen
                    onCheckedChanged: {
                        if (oceanuiData.sysItemModel.lockScreen !== checked) {
                            oceanuiData.sysItemModel.lockScreen = checked
                        }
                        // oceanuiData.sysItemModel.lockScreen = checked
                    }
                }
                D.Label {
                    text: qsTr("When the screen is locked")
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        onClicked: {
                            lockScreenCheckBox.checked = !lockScreenCheckBox.checked
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }
    OceanUIObject {
        name: "enableDoNotDisturb"
        parentName: "notification"
        displayName: qsTr("Number of notifications shown on the desktop")
        weight: 30
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: D.ComboBox {
            model: ["1", "2", "3"]
            flat: true
            currentIndex: oceanuiData.sysItemModel.bubbleCount - 1
            onCurrentIndexChanged: {
                if (oceanuiData.sysItemModel.bubbleCount - 1 !== currentIndex) {
                    oceanuiData.sysItemModel.bubbleCount = currentIndex + 1
                }
            }
        }
    }

    OceanUIObject {
        id: applicationList
        name: "applicationList"
        parentName: "notification"
        weight: 50
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIRepeater {
            model: oceanuiData.appListModel()
            delegate: OceanUIObject {
                name: "notificationItem" + index
                parentName: "applicationList"
                pageType: OceanUIObject.MenuEditor
                weight: 10 + index
                icon: model.AppIcon
                displayName: model.AppName
                // pageType: OceanUIObject.Item
                backgroundType: OceanUIObject.Normal
                page: D.Switch {
                    checked: model.EnableNotification
                    onCheckedChanged: {
                        if (model.EnableNotification !== checked) {
                            model.EnableNotification = checked
                        }
                    }
                }
                OceanUIObject{
                    name: "notificationItemDetails" + index
                    parentName: "applicationList/" + "notificationItem" + index
                    // pageType: OceanUIObject.Menu
                    OceanUIObject{
                        backgroundType: OceanUIObject.Normal
                        name: "allowNotifications" + index
                        parentName: "applicationList/notificationItem" + index + "/" + "notificationItemDetails" + index
                        displayName: qsTr("Allow Notifications")
                        description: qsTr("Display notification on desktop or show unread messages in the notification center")
                        icon: model.AppIcon
                        weight: 10
                        pageType: OceanUIObject.Editor
                        page: D.Switch {
                            Layout.rightMargin: 10
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            checked: model.EnableNotification
                            onCheckedChanged: {
                                if (model.EnableNotification !== checked) {
                                    model.EnableNotification = checked
                                }
                            }
                        }
                    }
                    OceanUIObject {
                        name: "notificationItemDetailsType" + index
                        parentName: "applicationList/notificationItem" + index + "/" + "notificationItemDetails" + index
                        backgroundType: OceanUIObject.Normal
                        visible: model.EnableNotification
                        weight: 20
                        pageType: OceanUIObject.Item
                        page: Rectangle {
                            color: "transparent"
                            implicitHeight: rowView.height + 20
                            RowLayout {
                                id: rowView
                                width: parent.width
                                anchors.centerIn: parent
                                ImageCheckBox {
                                    Layout.alignment: Qt.AlignCenter
                                    text: qsTr("Desktop")
                                    imageName: "notify_desktop"
                                    checked: model.ShowNotificationDesktop
                                    onCheckedChanged: {
                                        if (checked !== model.ShowNotificationDesktop) {
                                            model.ShowNotificationDesktop = checked
                                        }
                                    }
                                }
                                ImageCheckBox {
                                    Layout.alignment: Qt.AlignCenter
                                    text: qsTr("Lock Screen")
                                    imageName: "notify_lock"
                                    visible: false
                                    checked: model.LockScreenShowNotification
                                    onCheckedChanged: {
                                        if (checked !== model.LockScreenShowNotification) {
                                            model.LockScreenShowNotification = checked
                                        }
                                    }
                                }
                                ImageCheckBox {
                                    Layout.alignment: Qt.AlignCenter
                                    text: qsTr("Notification Center")
                                    imageName: "notify_center"
                                    checked: model.ShowNotificationCenter
                                    onCheckedChanged: {
                                        if (checked !== model.ShowNotificationCenter) {
                                            model.ShowNotificationCenter = checked
                                        }
                                    }
                                }
                            }
                        }
                    }
                    OceanUIObject {
                        name: "notificationSettingsGroup" + index
                        parentName: "applicationList/notificationItem" + index + "/" + "notificationItemDetails" + index
                        pageType: OceanUIObject.Item
                        backgroundType: OceanUIObject.Normal
                        visible: model.EnableNotification
                        weight: 30
                        page: OceanUIGroupView {}
                        OceanUIObject {
                            name: "notificationPreview" + index
                            parentName: "applicationList/notificationItem" + index + "/" + "notificationItemDetails" + index +"/"+"notificationSettingsGroup" + index
                            displayName: qsTr("Show message preview")
                            pageType: OceanUIObject.Item
                            weight: 10
                            page: D.CheckBox {
                                Layout.rightMargin: 10
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                text: oceanuiObj.displayName
                                checked: model.EnablePreview
                                onCheckedChanged: {
                                    if (model.EnablePreview !== checked) {
                                        model.EnablePreview = checked
                                    }
                                }
                            }
                        }
                        OceanUIObject {
                            name: "notificationSound" + index
                            parentName: "applicationList/notificationItem" + index + "/" + "notificationItemDetails" + index +"/"+"notificationSettingsGroup" + index
                            displayName: qsTr("Play a sound")
                            pageType: OceanUIObject.Item
                            weight: 20
                            page: D.CheckBox {
                                Layout.rightMargin: 10
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                text: oceanuiObj.displayName
                                checked: model.EnableSound
                                onCheckedChanged: {
                                    if (model.EnableSound !== checked) {
                                        model.EnableSound = checked
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
