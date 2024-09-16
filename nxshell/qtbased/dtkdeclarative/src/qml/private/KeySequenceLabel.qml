// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import ".."

Control {
    id: control
    property string text
    property D.Palette textColor: DS.Style.keySequenceEdit.label.text
    property D.Palette backgroundColor1: DS.Style.keySequenceEdit.label.background1
    property D.Palette backgroundColor2: DS.Style.keySequenceEdit.label.background2
    property D.Palette shadowInnerColor1: DS.Style.keySequenceEdit.label.shadowInner1
    property D.Palette shadowInnerColor2: DS.Style.keySequenceEdit.label.shadowInner2
    property D.Palette shadowOuterColor: DS.Style.keySequenceEdit.label.shadowOuter

    leftPadding: DS.Style.keySequenceEdit.label.leftRightPadding
    topPadding: DS.Style.keySequenceEdit.label.topBottomPadding
    rightPadding: leftPadding
    bottomPadding: topPadding

    contentItem: Text {
        text: control.text
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        font: D.DTK.fontManager.t8
        color: control.D.ColorSelector.textColor
    }

    background: Item {
        BoxShadow {
            anchors.fill: parent
            cornerRadius: backgroundRect.radius
            shadowOffsetY: 1
            shadowBlur: 2
            shadowColor: control.D.ColorSelector.shadowOuterColor;
            hollow: true
        }

        Rectangle {
            id: backgroundRect
            anchors.fill: parent

            radius: DS.Style.control.radius
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: control.D.ColorSelector.backgroundColor1
                }
                GradientStop {
                    position: 0.98
                    color: control.D.ColorSelector.backgroundColor2
                }
            }
        }

        BoxInsetShadow {
            anchors.fill: parent
            cornerRadius: backgroundRect.radius
            shadowOffsetY: -2
            shadowBlur: 1
            shadowColor: control.D.ColorSelector.shadowInnerColor1
        }

        BoxInsetShadow {
            visible: D.DTK.themeType === D.ApplicationHelper.DarkType
            anchors.fill: parent
            cornerRadius: backgroundRect.radius
            shadowOffsetY: 1
            shadowBlur: 1
            shadowColor: control.D.ColorSelector.shadowInnerColor2
        }
    }
}
