// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0 as D

Control {
    id: control
    width: 100; height: 100
    D.DciIcon.mode: D.DTK.NormalState
    D.DciIcon.theme: D.ApplicationHelper.LightType
    contentItem: D.DciIcon {
        name: "switch_button"
        mode: control.D.DciIcon.mode
        theme: control.D.DciIcon.theme
    }
    background: null
}
