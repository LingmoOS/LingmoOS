// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private as P

Button {
    id: control

    implicitWidth: Math.max(DS.Style.control.implicitWidth(control), DS.Style.control.implicitHeight(control))
    implicitHeight: implicitWidth

    topPadding: DS.Style.iconButton.padding
    bottomPadding: DS.Style.iconButton.padding
    leftPadding: DS.Style.iconButton.padding
    rightPadding: DS.Style.iconButton.padding
    icon {
        width: DS.Style.iconButton.iconSize
        height: DS.Style.iconButton.iconSize
        color: D.ColorSelector.textColor
    }

    contentItem: D.DciIcon {
        name: control.icon.name
        palette: D.DTK.makeIconPalette(control.palette)
        mode: control.D.ColorSelector.controlState
        theme: control.D.ColorSelector.controlTheme
        sourceSize: Qt.size(control.icon.width, control.icon.height)
    }

    background: P.ButtonPanel {
        implicitWidth: DS.Style.iconButton.backgroundSize
        implicitHeight: DS.Style.iconButton.backgroundSize
        button: control
    }
}
