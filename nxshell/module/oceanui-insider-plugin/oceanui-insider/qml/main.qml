// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0

OceanUITitleObject {
    name: "dmTitle"
    parentName: "insider"
    weight: 10
    displayName: qsTr("New Display Manager")

    Component {
        id: itemPage
        ItemDelegate {
            Layout.fillWidth: true
            text: oceanuiObj.displayName
            hoverEnabled: true
            checkable: false
            checked: true
            cascadeSelected: true
            backgroundVisible: false
            content: OceanUICheckIcon {
                visible: oceanuiData.currentItems.includes(oceanuiObj.name)
            }
            background: OceanUIItemBackground {
                separatorVisible: false
            }
            onClicked: {
                oceanuiData.setCurrentItem(oceanuiObj.name)
                console.log("onClicked", oceanuiObj.name, oceanuiData.currentItems, oceanuiData.currentItems.includes(oceanuiObj.name))
            }
        }
    }
    OceanUIObject {
        name: "selectDisplayManager"
        parentName: "insider"
        weight: 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "lightdm"
            parentName: "selectDisplayManager"
            weight: 10
            displayName: qsTr("Currently stable Display Manager (lightdm)")
            pageType: OceanUIObject.Item
            page: itemPage
        }
        OceanUIObject {
            name: "treeland"
            parentName: "selectDisplayManager"
            weight: 20
            displayName: qsTr("Technology preview Display Manager/Window Manager (ddm/treeland)")
            pageType: OceanUIObject.Item
            page: itemPage
        }
    }
    OceanUITitleObject {
        name: "imTitle"
        parentName: "insider"
        weight: 30
        visible: false
        displayName: qsTr("New Input Method")
    }
    OceanUIObject {
        name: "selectInputMethod"
        parentName: "insider"
        weight: 40
        visible: false
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "fcitx5"
            parentName: "selectInputMethod"
            weight: 10
            displayName: qsTr("Currently stable Input Method")
            pageType: OceanUIObject.Item
            page: itemPage
        }
        OceanUIObject {
            name: "lingmo-im"
            parentName: "selectInputMethod"
            weight: 20
            displayName: qsTr("Technology preview Input Method (lingmo-im)")
            pageType: OceanUIObject.Item
            page: itemPage
        }
    }
}
