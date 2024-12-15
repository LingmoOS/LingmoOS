// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.3

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    page: Control {
        id: control
        anchors.fill: parent
        contentItem: oceanuiObj.children.length > 0 ? oceanuiObj.children[0].getSectionItem(control) : null
    }

    LoginPage {
        visible: !oceanuiData.model.loginState
    }

    UserInfo {
        visible: oceanuiData.model.loginState
    }
}
