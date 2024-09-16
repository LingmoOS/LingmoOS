// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.MenuSeparator {
    id: control

    property string text
    property bool __lineStyle: control.text === ""
    property D.Palette textColor: DS.Style.menu.separatorText

    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentItem.implicitHeight + topPadding + bottomPadding)

    topPadding: __lineStyle ? DS.Style.menu.separator.lineTopPadding : DS.Style.menu.separator.topPadding
    bottomPadding: __lineStyle ? DS.Style.menu.separator.lineBottomPadding : DS.Style.menu.separator.bottomPadding

    contentItem: Loader {
        sourceComponent: __lineStyle ? lineCom : titleCom

        Component {
            id: lineCom
            Rectangle {
                property D.Palette separatorColor: DS.Style.menu.separator.lineColor
                width: control.parent.width
                implicitHeight: DS.Style.menu.separator.lineHeight
                color: D.ColorSelector.separatorColor
            }
        }

        Component {
            id: titleCom
            Text {
                text: control.text
                font: D.DTK.fontManager.t8
                color: control.D.ColorSelector.textColor
            }
        }
    }
}
