// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import "../"

Control {
    id: control

    property Component action: ActionButton {
        Layout.alignment: Qt.AlignRight
        visible: control.iconName

        onClicked: control.clicked()

        icon {
            height: 14
            name: control.iconName
            width: 14
        }
    }
    property int contrlImplicitWidth: 66
    property int contrlIntimplicitHeight: 40
    property int corners: RoundRectangle.NoneCorner
    property string description
    property int descriptionWidth: control.width - control.leftPadding - control.rightPadding
    property string iconName
    property Palette infoTextColor: Palette {
        normal: Qt.rgba(0, 0, 0, 1)
        normalDark: Qt.rgba(1, 1, 1, 1)
    }
    property Palette sectionTextColor: Palette {
        normal: Qt.rgba(0, 0, 0, 0.6)
        normalDark: Qt.rgba(1, 1, 1, 0.6)
    }
    property string title

    signal clicked

    bottomPadding: 4
    implicitWidth: contrlImplicitWidth
    leftPadding: 10
    rightPadding: 10
    topPadding: 3

    background: RoundRectangle {
        color: Qt.rgba(0, 0, 0, 0.05)
        corners: control.corners
        implicitHeight: contrlIntimplicitHeight
        implicitWidth: contrlImplicitWidth
        radius: Style.control.radius
    }
    contentItem: ColumnLayout {
        spacing: 0

        ElideLabel {
            Layout.fillWidth: true
            // 系数微调整以满足默认字号标签均显示的效果
            Layout.minimumWidth: descriptionWidth + 5
            color: control.ColorSelector.sectionTextColor
            font: DTK.fontManager.t10
            sourceText: control.title
            tipsColor: control.palette.toolTipText
        }

        RowLayout {
            id: content

            ElideLabel {
                Layout.fillWidth: true
                color: control.ColorSelector.infoTextColor
                sourceText: control.description
                tipsColor: control.palette.toolTipText
                width: descriptionWidth
            }

            Loader {
                Layout.rightMargin: 5
                sourceComponent: control.action
            }
        }
    }
}
