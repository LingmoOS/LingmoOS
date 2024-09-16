// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Control {
    id: control

    default property alias buttons: btnGroup.buttons
    property alias group: btnGroup

    D.ColorSelector.hovered: false
    padding: DS.Style.buttonBox.padding

    ButtonGroup {
        id: btnGroup
    }
    contentItem: RowLayout {
        spacing: DS.Style.buttonBox.spacing
        children: control.buttons
    }

    background: BoxPanel {
        implicitWidth: DS.Style.buttonBox.width
        implicitHeight: DS.Style.buttonBox.height
        backgroundFlowsHovered: D.ColorSelector.family === D.Palette.CrystalColor
        outsideBorderColor: null
        color2: color1
    }
}
