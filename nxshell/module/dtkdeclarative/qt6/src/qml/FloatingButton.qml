// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS
import org.lingmo.dtk.private 1.0 as P

Button {
    id: control

    implicitWidth: Math.max(DS.Style.control.implicitWidth(control), DS.Style.control.implicitHeight(control))
    implicitHeight: implicitWidth
    checkable: false
    checked: true
    background: P.ButtonPanel {
        implicitWidth: DS.Style.floatingButton.size
        implicitHeight: DS.Style.floatingButton.size
        button: control
        radius: control.width / 2
    }
}
