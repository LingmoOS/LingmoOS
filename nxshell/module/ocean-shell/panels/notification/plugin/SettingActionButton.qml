// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.lingmo.dtk 1.0
import org.lingmo.ds.notification

ActionButton {
    id: root

    property int radius: 15
    property bool forcusBorderVisible: root.visualFocus

    icon.width: 16
    icon.height: 16
    padding: 4
    font: DTK.fontManager.t8

    Loader {
        anchors.fill: parent
        active: root.forcusBorderVisible

        sourceComponent: FocusBoxBorder {
            radius: root.radius
            color: root.palette.highlight
        }
    }

    background: BoxPanel {
        radius: root.radius
        enableBoxShadow: true
        boxShadowBlur: 10
        boxShadowOffsetY: 4
        color1: Palette {
            normal {
                common: ("transparent")
                crystal: Qt.rgba(240 / 255.0, 240 / 255.0, 240 / 255.0, 0.5)
            }
            normalDark {
                crystal: Qt.rgba(24 / 255.0, 24 / 255.0, 24 / 255.0, 0.5)
            }
        }

        color2: color1
        insideBorderColor: Palette {
            normal {
                common: ("transparent")
                crystal: Qt.rgba(255 / 255.0, 255 / 255.0, 255 / 255.0, 0.2)
            }
            normalDark {
                crystal: Qt.rgba(255 / 255.0, 255 / 255.0, 255 / 255.0, 0.1)
            }
        }
        outsideBorderColor: Palette {
            normal {
                common: ("transparent")
                crystal: Qt.rgba(0, 0, 0, 0.08)
            }
            normalDark {
                crystal: Qt.rgba(0, 0, 0, 0.4)
            }
        }
        dropShadowColor: Palette {
            normal {
                common: ("transparent")
                crystal: Qt.rgba(0, 0, 0, 0.2)
            }
            normalDark {
                crystal: Qt.rgba(0, 0, 0, 0.4)
            }
        }
        innerShadowColor1: null
        innerShadowColor2: innerShadowColor1
    }
}
