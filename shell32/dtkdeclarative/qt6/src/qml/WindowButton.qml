// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Control {
    id: control
    property alias icon: iconLoader
    readonly property bool pressed: mouseArea.pressed
    signal clicked(var mouse)
    property D.Palette textColor: DS.Style.button.text
    property D.Palette backgroundColor: DS.Style.windowButton.background

    palette.windowText: D.ColorSelector.textColor
    hoverEnabled: true
    contentItem: D.DciIcon {
        id: iconLoader
        palette: D.DTK.makeIconPalette(control.palette)
        sourceSize {
            width: DS.Style.windowButton.width
            height: DS.Style.windowButton.height
        }
        mode: control.D.ColorSelector.controlState
        theme: control.D.ColorSelector.controlTheme
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        Component.onCompleted: clicked.connect(control.clicked)
    }
    background: Rectangle {
        implicitWidth: DS.Style.windowButton.width
        implicitHeight: DS.Style.windowButton.height
        color: control.D.ColorSelector.backgroundColor
    }
}
