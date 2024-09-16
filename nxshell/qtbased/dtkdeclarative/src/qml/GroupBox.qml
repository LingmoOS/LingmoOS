// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.style 1.0 as DS

T.GroupBox {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            label ? label.implicitWidth + leftPadding + rightPadding : 0,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentHeight + topPadding + bottomPadding)

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    padding: 0
    topPadding: padding + (label && label.implicitWidth > 0 ? label.implicitHeight + spacing : 0)

    label: Label {
        x: DS.Style.control.radius
        text: control.title
        color: control.palette.windowText
        elide: Text.ElideRight
        verticalAlignment: Text.AlignLeft

        background: Rectangle { }
    }

    background: Rectangle {
        y: label.height / 2
        width: parent.width
        radius: DS.Style.control.radius

        color: control.palette.window
    }
}
