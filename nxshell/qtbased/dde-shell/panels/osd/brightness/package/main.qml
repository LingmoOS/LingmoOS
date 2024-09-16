// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

AppletItem {
    id: control
    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height

    function update(osdType)
    {
        if (match(osdType)) {
            Applet.sync()
            return true
        }
        return false
    }

    function match(osdType)
    {
        return types.indexOf(osdType) >= 0
    }

    property var types: [
        "BrightnessUp",
        "BrightnessDown",
        "BrightnessUpAsh",
        "BrightnessDownAsh"
    ]

    RowLayout {
        anchors.leftMargin: 68
        anchors.rightMargin: 26

        D.DciIcon {
            sourceSize {
                width: 32
                height: 32
            }
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 14
            name: Applet.iconName
        }

        D.ProgressBar {
            Layout.preferredWidth: 200
            from: 0
            value: Applet.brightness
            to: 1
        }

        Text {
            font: D.DTK.fontManager.t4
            text: Number(Applet.brightness * 100).toFixed(0) + "%"
        }
    }
}
