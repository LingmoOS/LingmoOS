// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root

    name: "userinfo"
    parentName: "lingmoid"
    pageType: OceanUIObject.Item
    page: OceanUISettingsView {}
    OceanUIObject {
        name: "body"
        parentName: "lingmoid/userinfo"
        weight: 10
        pageType: OceanUIObject.Item

        OceanUIObject {
            name: "title"
            parentName: "lingmoid/userinfo/body"
            weight: 10
            pageType: OceanUIObject.Item
            page: ColumnLayout {
                id: titleLayout
                Image {
                    id: userIcon
                    Layout.alignment: Qt.AlignHCenter
                    source: oceanuiData.model.avatar
                    sourceSize: Qt.size(80, 80)
                    fillMode: Image.PreserveAspectFit
                    clip: true
                }

                // // fake round image
                // D.BoxShadow {
                //     id: boxShadow
                //     hollow: true
                //     anchors.fill: userIcon
                //     shadowBlur: 10
                //     spread: 20
                //     shadowColor: titleLayout.palette.window
                //     cornerRadius: boxShadow.width / 2
                // }

                Label {
                    id: userRegion
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: oceanuiData.model.region
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: oceanuiData.model.displayName
                    }
                }
            }
        }

        OceanUIObject {
            name: "syncService"
            parentName: "lingmoid/userinfo/body"
            weight: 15
            pageType: OceanUIObject.Item
            page: Rectangle {
                radius: 20
                implicitHeight: 300
                color: "#FFFFFF"

                Image {
                    id: waitImg
                    anchors.verticalCenterOffset: -20
                    anchors.centerIn: parent
                    source: "qrc:/icons/lingmo/builtin/icons/waitting.svg"
                    sourceSize: Qt.size(40, 40)
                    fillMode: Image.PreserveAspectFit
                    clip: true
                }

                Label {
                    anchors.top: waitImg.bottom
                    anchors.topMargin: 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("The relevant functions are still under development, so stay tuned")
                }
            }
        }

        OceanUITitleObject {
            name: "syncServiceTitle"
            parentName: "lingmoid/userinfo/body"
            displayName: qsTr("Synchronization Service")
            weight: 20
            visible: false
        }

        OceanUIObject {
            name: "syncService"
            parentName: "lingmoid/userinfo/body"
            weight: 30
            pageType: OceanUIObject.Item
            page: OceanUIGroupView {}

            OceanUIObject {
                name: "syncServiceSwitch"
                parentName: "lingmoid/userinfo/body/syncService"
                displayName: qsTr("%1 Auto Sync").arg(oceanuiData.editionName())
                description: qsTr("Securely store system settings and personal data in the cloud, and keep them in sync across devices")
                weight: 10
                pageType: OceanUIObject.Editor
                page: Switch {

                }
                visible: false
            }
        }
    }

    OceanUIObject {
        name: "footer"
        parentName: "lingmoid/userinfo"
        weight: 20
        pageType: OceanUIObject.Item
        OceanUIObject {
            name: "delete"
            parentName: "lingmoid/userinfo/footer"
            weight: 10
            pageType: OceanUIObject.Item
            page: Button {
                text: qsTr("Sign out")
                onClicked: {
                    oceanuiData.worker.logoutUser()
                }
            }
        }
        OceanUIObject {
            // 按钮区域可加个空项处理右对齐问题
            name: "spacer"
            parentName: "lingmoid/userinfo/footer"
            weight: 20
            pageType: OceanUIObject.Item
            page: Item {
                Layout.fillWidth: true
            }
        }
        OceanUIObject {
            name: "cancel"
            parentName: "lingmoid/userinfo/footer"
            weight: 30
            pageType: OceanUIObject.Item
            page: Button {
                text: qsTr("Go to web settings")
                onClicked: {
                    oceanuiData.worker.openWeb()
                }
            }
        }
    }
}
