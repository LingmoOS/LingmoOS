// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    name: "lingmoid"
    parentName: "root"
    displayName: OceanUIApp.uosEdition() === OceanUIApp.UosCommunity ? qsTr("lingmo ID") : qsTr("UOS ID")
    description: qsTr("Cloud services")
    icon: "lingmoid"
    weight: 70
    
    visible: false
    OceanUIDBusInterface {
        property var isLogin
        service: "com.lingmo.lingmoid"
        path: "/com/lingmo/lingmoid"
        inter: "com.lingmo.lingmoid"
        connection: OceanUIDBusInterface.SessionBus
        onIsLoginChanged: {
            root.visible = true
        }
    }
}
