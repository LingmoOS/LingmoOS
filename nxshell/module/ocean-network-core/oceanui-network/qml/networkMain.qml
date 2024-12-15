// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0

OceanUIObject {
    id: root
    property var wiredDevs: []
    property var wirelessDevs: []

    function showPage(cmd) {
        oceanuiData.exec(NetManager.ShowPage, cmd, {})
    }
    function updateDevice() {
        const delWiredDevs = wiredDevs.concat()
        const delWirelessDevs = wirelessDevs.concat()
        for (let i in oceanuiData.root.children) {
            let item = oceanuiData.root.children[i]
            switch (item.itemType) {
            case NetType.WiredDeviceItem:
            {
                let index = delWiredDevs.findIndex(d => d.item === item)
                if (index >= 0) {
                    delWiredDevs.splice(index, 1)
                } else {
                    let dev = wiredComponent.createObject(root, {
                                                              "item": item
                                                          })
                    OceanUIApp.addObject(dev)
                    wiredDevs.push(dev)
                }
            }
            break
            case NetType.WirelessDeviceItem:
            {
                let index = delWirelessDevs.findIndex(d => d.item === item)
                if (index >= 0) {
                    delWirelessDevs.splice(index, 1)
                } else {
                    let dev = wirelessComponent.createObject(root, {
                                                                 "item": item,
                                                                 "airplaneItem": oceanuiData.root
                                                             })
                    OceanUIApp.addObject(dev)
                    wirelessDevs.push(dev)
                }
            }
            break
            case NetType.VPNControlItem:
                if (vpnPage.item !== item) {
                    vpnPage.item = item
                }
                break
            case NetType.SystemProxyControlItem:
                if (systemProxyPage.item !== item) {
                    systemProxyPage.setItem(item)
                }
                break
            case NetType.AppProxyControlItem:
                if (appProxyPage.item !== item) {
                    appProxyPage.item = item
                }
                break
            case NetType.HotspotControlItem:
                hotspotPage.setItem(item)
                break
            case NetType.DSLControlItem:
                if (dslPage.item !== item) {
                    dslPage.item = item
                }
                break
            case NetType.DetailsItem:
                if (detailsPage.item !== item) {
                    detailsPage.item = item
                }
                break
            }
        }
        for (const delDev of delWiredDevs) {
            OceanUIApp.removeObject(delDev)
            let index = wiredDevs.findIndex(item => delDev === item)
            if (index >= 0) {
                wiredDevs.splice(index, 1)
            }
            delDev.destroy()
        }
        for (const delDev of delWirelessDevs) {
            OceanUIApp.removeObject(delDev)
            let index = wirelessDevs.findIndex(item => delDev === item)
            if (index >= 0) {
                wirelessDevs.splice(index, 1)
            }
            delDev.destroy()
        }
    }
    Component {
        id: wiredComponent
        PageWiredDevice {
            property var showPage: root.showPage
        }
    }
    Component {
        id: wirelessComponent
        PageWirelessDevice {
            property var showPage: root.showPage
        }
    }

    PageVPN {
        id: vpnPage
        property var showPage: root.showPage
        name: "networkVpn"
        parentName: "network"
        weight: 3010
    }
    PageSystemProxy {
        id: systemProxyPage
        property var showPage: root.showPage
        name: "systemProxy"
        parentName: "network"
        weight: 3020
    }
    PageAppProxy {
        id: appProxyPage
        property var showPage: root.showPage
        name: "applicationProxy"
        parentName: "network"
        weight: 3030
    }
    PageHotspot {
        id: hotspotPage
        property var showPage: root.showPage
        name: "personalHotspot"
        parentName: "network"
        weight: 3040
    }
    PageAirplane {
        name: "airplaneMode"
        property var showPage: root.showPage
        parentName: "network"
        weight: 3050
        item: oceanuiData.root
    }
    PageDSL {
        id: dslPage
        property var showPage: root.showPage
        name: "dsl"
        parentName: "network"
        weight: 3060
    }
    PageDetails {
        id: detailsPage
        property var showPage: root.showPage
        name: "networkDetails"
        parentName: "network"
        weight: 3070
    }
    Connections {
        target: oceanuiData.root
        function onChildrenChanged() {
            updateDevice()
        }
    }
    Component.onCompleted: {
        updateDevice()
    }
}
