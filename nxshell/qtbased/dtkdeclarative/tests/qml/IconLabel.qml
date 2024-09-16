// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D

Rectangle {
    id: control

    width: 100; height: 100

    property QtObject icon: QtObject{
        property string name: "switch_button"
        property int width: 20
        property int height: 20
    }

    D.IconLabel {
        anchors.centerIn: parent
        width: 50; height: 50
        text: "test"
        font: D.DTK.fontManager.t5
        color: "red"
        display: D.IconLabel.IconBesideText
        spacing: 2
        mirrored: false
        alignment: Qt.AlignLeft
        topPadding: 1
        leftPadding: 1
        rightPadding: 1
        bottomPadding: 1
        icon: D.DTK.makeIcon(control.icon, control.D.DciIcon)
    }
}
