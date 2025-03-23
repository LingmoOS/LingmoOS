// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

ToolTip {
    id: control
    property Item target

    x: 0
    topPadding: DS.Style.alertToolTip.verticalPadding
    bottomPadding: DS.Style.alertToolTip.verticalPadding
    leftPadding: DS.Style.alertToolTip.horizontalPadding
    rightPadding: DS.Style.alertToolTip.horizontalPadding
    implicitWidth: Math.min(DS.Style.control.implicitWidth(control), target.width)
    implicitHeight: DS.Style.control.implicitHeight(control)
    margins: 0
    closePolicy: Popup.NoAutoClose

    background: Item {
        BoxShadow {
            anchors.fill: _background
            shadowBlur: 20
            shadowOffsetY: 6
            shadowColor: Qt.rgba(0, 0, 0, 0.2)
            cornerRadius: _background.radius
        }

        Rectangle {
            property D.Palette backgroundColor: DS.Style.alertToolTip.background
            property D.Palette borderColor: DS.Style.control.border
            id: _background
            anchors.fill: parent
            color: D.ColorSelector.backgroundColor
            border.color: D.ColorSelector.borderColor
            radius: DS.Style.control.radius
        }
    }

    contentItem: Text {
        property D.Palette textColor: DS.Style.alertToolTip.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        text: control.text
        font: control.font
        color: D.ColorSelector.textColor
        wrapMode: Text.Wrap
    }

    enter: Transition {
        NumberAnimation { properties: "opacity"; from: 0.0; to: 1.0; duration: 200 }
        NumberAnimation { properties: "y"; from: control.target.height; to: control.target.height + DS.Style.control.spacing; duration: 200 }
    }

    exit: Transition {
        NumberAnimation { properties: "opacity"; from: 1.0; to: 0.0 }
        NumberAnimation { properties: "y"; from: control.target.height + DS.Style.control.spacing ; to: control.target.height }
    }

    BoxShadow {
        property D.Palette dropShadowColor: DS.Style.alertToolTip.connecterdropShadow
        property D.Palette backgroundColor: DS.Style.alertToolTip.connecterBackground
        y: - height * (0.75) - control.topMargin - control.topPadding
        width: DS.Style.alertToolTip.connectorWidth
        height: DS.Style.alertToolTip.connectorHeight
        shadowBlur: 4
        shadowOffsetY: 2
        shadowColor: D.ColorSelector.dropShadowColor
        cornerRadius: _background.radius

        Rectangle {
            anchors.fill: parent
            color: parent.D.ColorSelector.backgroundColor
            border.color: _background.D.ColorSelector.borderColor
            border.width: 1
        }
    }
}
