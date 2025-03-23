// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.Button {
    id: control
    property D.Palette textColor: DS.Style.button.text

    palette.windowText: pressed ? D.ColorSelector.textColor : undefined
    opacity: D.ColorSelector.controlState === D.DTK.DisabledState ? 0.4 : 1
    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    icon {
        width: DS.Style.button.iconSize
        height: DS.Style.button.iconSize
    }
    contentItem: D.DciIcon {
        palette: D.DTK.makeIconPalette(control.palette)
        mode: control.D.ColorSelector.controlState
        theme: control.D.ColorSelector.controlTheme
        name: control.icon.name
        sourceSize: Qt.size(control.icon.width, control.icon.height)
    }
    background: null
}
