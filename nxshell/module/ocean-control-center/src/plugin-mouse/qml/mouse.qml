// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
import org.lingmo.oceanui 1.0

OceanUIObject {
    id: mouse
    name:"MouseAndTouchpad"
    parentName:"device"
    displayName: qsTr("Mouse and Touchpad")
    description: qsTr("Common、Mouse、Touchpad")
    icon:"oceanui_nav_mouse"
    weight: 30

    visible: !OceanUIApp.isTreeland()
}
