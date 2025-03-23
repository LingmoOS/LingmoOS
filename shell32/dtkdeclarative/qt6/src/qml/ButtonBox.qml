// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private 1.0 as P

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

    Loader {
        active: D.DTK.hasAnimation
        sourceComponent: P.ButtonPanel {
            id: backgroundPanel
            visible: btnGroup.checkedButton
            implicitWidth: visible ? btnGroup.checkedButton.width : 0
            implicitHeight: visible ? btnGroup.checkedButton.height : 0
            button: control
            outsideBorderColor: null
            color1: D.Palette {
                normal {
                    common: Qt.rgba(0, 0, 0, 0.2)
                }
            }
            color2: color1

            ParallelAnimation {
                running: btnGroup.checkedButton
                         && (btnGroup.checkedButton.x !== backgroundPanel.x
                             || btnGroup.checkedButton.y !== backgroundPanel.y)
                NumberAnimation {
                    target: backgroundPanel
                    property: "x"
                    to: btnGroup.checkedButton ? btnGroup.checkedButton.x : backgroundPanel.x
                    duration: 200
                }
                NumberAnimation {
                    target: backgroundPanel
                    property: "y"
                    to: btnGroup.checkedButton ? btnGroup.checkedButton.y : backgroundPanel.y
                    duration: 200
                }
                NumberAnimation {
                    target: backgroundPanel
                    property: "width"
                    to: btnGroup.checkedButton ? btnGroup.checkedButton.width : backgroundPanel.width
                    duration: 200
                }
                NumberAnimation {
                    target: backgroundPanel
                    property: "height"
                    to: btnGroup.checkedButton ? btnGroup.checkedButton.height : backgroundPanel.height
                    duration: 200
                }
            }
        }
    }
}
