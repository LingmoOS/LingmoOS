// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    id: root

    property var parentView

    name: "login"
    parentName: "lingmoid"
    pageType: OceanUIObject.Item
    page: Flickable {
        contentHeight: groupView.height
        ScrollBar.vertical: ScrollBar {
            width: 10
        }

        OceanUIGroupView {
            id: groupView
            isGroup: false
            height: implicitHeight + 10
            anchors {
                left: parent.left
                right: parent.right
            }
        }

        Component.onCompleted: {
            root.parentView = this
        }
    }
    OceanUIObject {
        parentName: "lingmoid/login"
        weight: 10
        pageType: OceanUIObject.Item
        page: Rectangle {
            implicitHeight: root.parentView ? root.parentView.height * 0.5 : 350
            color: "transparent"

            Image {
                id: bgImage
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                fillMode: Image.Pad
                clip: true                
                source: "qrc:/icons/lingmo/builtin/icons/oceanui_login_bg.svg"
            }

            Image {
                id: logoImage
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                fillMode: Image.Pad
                source: "qrc:/icons/lingmo/builtin/icons/oceanui_cloud_logo.svg"
            }
        }

        onParentItemChanged: {
            if (parentItem) {
                parentItem.bottomPadding = 20
            }
        }
    }

    OceanUIObject {
        parentName: "lingmoid/login"
        weight: 20
        pageType: OceanUIObject.Item
        page: RowLayout {
            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 30 
                font.bold: true
                text: qsTr("Cloud Sync")
            }
        }

        onParentItemChanged: {
            if (parentItem) {
                parentItem.bottomPadding = 20
            }
        }
    }

    OceanUIObject {
        parentName: "lingmoid/login"
        weight: 30
        pageType: OceanUIObject.Item
        page: RowLayout{
            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Manage your %1 ID and sync your personal data across devices.\nSign in to %1 ID to get personalized features and services of Browser, App Store, and more.").arg(oceanuiData.editionName())
            }
        }

        onParentItemChanged: {
            if (parentItem) {
                parentItem.bottomPadding = 20
            }
        }        
    }
    
    OceanUIObject {
        parentName: "lingmoid/login"
        weight: 40
        pageType: OceanUIObject.Item
        page: RowLayout{
            Button {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 30
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Sign In to %1 ID").arg(oceanuiData.editionName())
                onClicked: {
                    oceanuiData.worker.loginUser()
                }
            } 
        }
    }
}
