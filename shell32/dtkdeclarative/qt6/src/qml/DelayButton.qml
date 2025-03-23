// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls.impl 2.4
import QtQuick.Templates as T
import org.deepin.dtk.style 1.0 as DS

T.DelayButton {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             Math.max(contentItem.implicitHeight,
                                      indicator ? indicator.implicitHeight : 0) + topPadding + bottomPadding)

    padding: DS.Style.control.padding
    spacing: DS.Style.control.spacing

    transition: Transition {
        NumberAnimation {
            duration: control.delay * (control.pressed ? 1.0 - control.progress : 0.3 * control.progress)
        }
    }

    contentItem: ItemGroup {
        ClippedText {
            clip: control.progress > 0
            clipX: -control.leftPadding + control.progress * control.width
            clipWidth: (1.0 - control.progress) * control.width
            visible: control.progress < 1

            text: control.text
            font: control.font
            color: control.palette.buttonText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        ClippedText {
            clip: control.progress > 0
            clipX: -control.leftPadding
            clipWidth: control.progress * control.width
            visible: control.progress > 0

            text: control.text
            font: control.font
            color: control.palette.highlightedText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Rectangle {
        implicitWidth: control.text.length ? DS.Style.control.button.height + (4 * DS.Style.control.radius) : DS.Style.control.button.height + (2 * DS.Style.control.radius)
        implicitHeight: DS.Style.control.button.height
        radius: DS.Style.control.radius
        color: control.palette.button

       FocusBoxBorder {
            visible: control.visualFocus
            color: control.palette.highlight
            radius: parent.radius
            anchors.fill: parent
        }

        Item {
            width: control.progress * parent.width
            height: parent.height
            clip: true

            Rectangle {
                width: parent.parent.width
                height: parent.parent.height
                radius: DS.Style.control.radius
                color: control.palette.highlight
            }
        }
    }
}
