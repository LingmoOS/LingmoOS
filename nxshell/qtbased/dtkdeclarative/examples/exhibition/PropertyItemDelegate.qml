// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Control {
    id: control
    property string title
    property string description
    property int corners: RoundRectangle.NoneCorner
    property string iconName
    signal clicked()
    property Component action: ActionButton {
        visible: control.iconName
        Layout.alignment: Qt.AlignRight
        icon {
            width: 14
            height: 14
            name: control.iconName
        }
        onClicked: control.clicked()
    }
    padding: 5
    contentItem: ColumnLayout {
        Label {
            visible: control.title
            text: control.title
            font: DTK.fontManager.t10
        }
        RowLayout {
            Label {
                visible: control.description
                Layout.fillWidth: true
                text: control.description
                font: DTK.fontManager.t8
                elide: Text.ElideMiddle
            }
            Loader {
                Layout.leftMargin: 5
                sourceComponent: control.action
            }
        }
    }

    background: RoundRectangle {
        implicitWidth: 66
        implicitHeight: 40
        color:  Qt.rgba(0, 0, 0, 0.05)
        radius: Style.control.radius
        corners: control.corners
    }
}
