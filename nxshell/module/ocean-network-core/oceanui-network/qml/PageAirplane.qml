// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import Qt.labs.qmlmodels 1.2

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0

OceanUIObject {
    id: root
    property var item: null

    displayName: qsTr("Airplane mode")
    description: qsTr("Stop wireless communication")
    icon: "oceanui_airplane_mode"
    visible: item && item.enabledable
    backgroundType: OceanUIObject.Normal
    pageType: OceanUIObject.MenuEditor
    page: devCheck
    Component {
        id: devCheck
        D.Switch {
            checked: item.isEnabled
            enabled: item.enabledable
            onClicked: {
                oceanuiData.exec(NetManager.SetConnectInfo, item.id, {
                                 "enable": checked
                             })
            }
        }
    }

    OceanUIObject {
        name: "menu"
        parentName: root.name
        OceanUIObject {
            name: "title"
            parentName: root.name + "/menu"
            displayName: root.displayName
            icon: "oceanui_airplane_mode"
            weight: 10
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: devCheck
        }
        OceanUIObject {
            name: "airplaneTips"
            parentName: root.name + "/menu"
            displayName: qsTr("Enabling the airplane mode turns off wireless network, personal hotspot and Bluetooth")
            weight: 20
            pageType: OceanUIObject.Item
            page: D.Label {
                text: oceanuiObj.displayName
                wrapMode: Text.WordWrap
            }
        }
    }
}
