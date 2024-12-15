// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// import org.lingmo.dtk 1.0 as D
import QtQuick 2.15
import QtQuick.Controls 2.0
import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui 1.0
import QtQuick.Layouts 1.15

OceanUIObject {
    OceanUIObject {
        name: "checkUpdate"
        parentName: "update"
        backgroundType: OceanUIObject.AutoBg
        displayName: qsTr("check update")
        description: oceanuiData.model().upgradable ? qsTr("Your system is already the latest version") : qsTr("You have a new system update, please check and update")
        pageType: OceanUIObject.Menu
        weight: 10
        visible: true
        page: OceanUIGroupView {
            isGroup: false
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 60
                rightMargin: 60
            }

            CheckUpdate {
                id: checkUpdate
                visible: !oceanuiData.model().showUpdateCtl
            }

            Component.onCompleted: {
                console.log(" checkUpdate : ", oceanuiData.model().upgradable)
                oceanuiData.work().checkUpgrade();
            }

        }

        UpdateControl {
        }

    }

    OceanUIObject {
        visible: false

        name: "updateSettings"
        parentName: "update"
        displayName: qsTr("Update Settings")
        description: qsTr("You can set system updates, security updates, idle updates, update reminders, etc.")
        icon: "update"
        weight: 40
    }

}
