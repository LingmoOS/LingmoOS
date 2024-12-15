// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import org.lingmo.oceanui 1.0

// 该文件中不能使用oceanuiData,根对象为OceanUIObject
OceanUIObject {
    id: root
    name: "example"
    parentName: "root"
    displayName: qsTr("Example")
    icon: "oceanui_example"
    weight: 1000

    visible: false // 控制模块显示
    OceanUIDBusInterface {
        property var windowRadius
        service: "org.lingmo.ocean.Appearance1"
        path: "/org/lingmo/ocean/Appearance1"
        inter: "org.lingmo.ocean.Appearance1"
        connection: OceanUIDBusInterface.SessionBus
        onWindowRadiusChanged: {
            root.visible = windowRadius > 0
        }
    }
}
