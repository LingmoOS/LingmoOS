// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// import org.lingmo.dtk 1.0 as D
import QtQuick 2.15
import QtQuick.Controls 2.0

import org.lingmo.oceanui 1.0
import QtQuick.Layouts 1.15

OceanUIObject {
    OceanUIObject {
        name: "bootMenu"
        parentName: "system"
        displayName: qsTr("Boot Menu")
        description: qsTr("Manage your boot menu")
        icon: "meau"
        weight: 80
        BootPage{}
    }
    OceanUIObject {
        name: "developerMode"
        parentName: "system"
        displayName: qsTr("Developer Mode")
        description: qsTr("Developer root permission management")
        //visible: !oceanuiData.mode().isCommunitySystem()
        visible: false
        icon: "developer"
        weight: 90
        DevelopModePage{}
    }
}
