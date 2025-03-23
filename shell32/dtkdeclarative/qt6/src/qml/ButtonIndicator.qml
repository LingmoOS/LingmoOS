// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Rectangle {
    property D.Palette backgroundColor: DS.Style.buttonIndicator.background
    property Item control: D.ColorSelector.control

    implicitWidth: DS.Style.buttonIndicator.size
    implicitHeight: implicitHeight
    radius: width / 2
    color: D.ColorSelector.backgroundColor

    D.DciIcon {
        anchors.centerIn: parent
        sourceSize {
            width: DS.Style.buttonIndicator.iconSize
        }
        name: "arrow_ordinary_down"
        palette: control.D.DTK.makeIconPalette(control.palette)
        mode: control.D.ColorSelector.controlState
        theme: control.D.ColorSelector.controlTheme
        fallbackToQIcon: false
    }
}
