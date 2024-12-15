// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0

OceanUITitleObject {
    id: root
    property var config: null
    property bool mppe: false

    property string errorKey: ""
    signal editClicked

    function setConfig(c) {
        errorKey = ""
        root.config = c !== undefined ? c : {}
        mppe = root.config.hasOwnProperty("require-mppe") && root.config["require-mppe"]
        root.configChanged()
    }
    function getConfig() {
        root.config["require-mppe"] = mppe
        if (!mppe) {
            delete root.config["require-mppe-128"]
            delete root.config["mppe-stateful"]
        }
        return root.config
    }
    function checkInput() {
        return true
    }
    name: "pppTitle"
    displayName: qsTr("PPP")
    Component {
        id: switchItem
        D.Switch {
            checked: root.config.hasOwnProperty(oceanuiObj.name) && root.config[oceanuiObj.name]
            onClicked: {
                root.config[oceanuiObj.name] = checked
                root.editClicked()
            }
        }
    }
    OceanUIObject {
        name: "pppGroup"
        parentName: root.parentName
        weight: root.weight + 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "useMPPE"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("Use MPPE")
            weight: 10
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: mppe
                onClicked: {
                    mppe = checked
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "require-mppe-128"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("128-bit MPPE")
            weight: 20
            visible: mppe
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "mppe-stateful"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("Stateful MPPE")
            weight: 30
            visible: mppe
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-eap"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("Refuse EAP Authentication")
            weight: 40
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-pap"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("Refuse PAP Authentication")
            weight: 50
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-chap"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("Refuse CHAP Authentication")
            weight: 60
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-mschap"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("Refuse MSCHAP Authentication")
            weight: 70
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-mschapv2"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("Refuse MSCHAPv2 Authentication")
            weight: 80
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "nobsdcomp"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("No BSD Data Compression")
            weight: 90
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "nodeflate"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("No Deflate Data Compression")
            weight: 100
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "no-vj-comp"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("No TCP Header Compression")
            weight: 110
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        // OceanUIObject {
        //     name: "nopcomp"
        //     parentName: root.parentName + "/pppGroup"
        //     displayName: qsTr("No Protocol Field Compression")
        //     weight: 120
        //     pageType: OceanUIObject.Editor
        //     page: switchItem
        // }
        // OceanUIObject {
        //     name: "noaccomp"
        //     parentName: root.parentName + "/pppGroup"
        //     displayName: qsTr("No Address/Control Compression")
        //     weight: 130
        //     pageType: OceanUIObject.Editor
        //     page:switchItem
        // }
        OceanUIObject {
            name: "lcpEchoInterval"
            parentName: root.parentName + "/pppGroup"
            displayName: qsTr("Send PPP Echo Packets")
            weight: 140
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.config["lcp-echo-failure"] === 5 && root.config["lcp-echo-interval"] === 30
                onClicked: {
                    if (checked) {
                        root.config["lcp-echo-failure"] = 5
                        root.config["lcp-echo-interval"] = 30
                    } else {
                        root.config["lcp-echo-failure"] = 0
                        root.config["lcp-echo-interval"] = 0
                    }
                    root.editClicked()
                }
            }
        }
    }
}
