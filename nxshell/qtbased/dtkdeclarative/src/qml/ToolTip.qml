// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.ToolTip {
    id: control

    x: parent ? (parent.width - implicitWidth) / 2 : 0
    y: -implicitHeight - 3

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)

    topPadding: DS.Style.toolTip.verticalPadding
    bottomPadding: topPadding
    leftPadding: DS.Style.toolTip.horizontalPadding
    rightPadding: leftPadding
    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent

    contentItem: Text {
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        text: control.text
        font: control.font
        wrapMode: Text.WordWrap
        opacity: enabled ? 1.0 : 0.4
        color: control.palette.toolTipText
    }

    background: FloatingPanel {
        implicitWidth: 0
        implicitHeight: DS.Style.toolTip.height
        radius: DS.Style.control.radius
    }
}
