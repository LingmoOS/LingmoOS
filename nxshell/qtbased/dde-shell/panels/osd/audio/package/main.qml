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
        "AudioUp",
        "AudioDown",
        "AudioMute",
        "AudioUpAsh",
        "AudioDownAsh",
        "AudioMuteAsh"
    ]

    RowLayout {
        anchors.leftMargin: 68
        anchors.rightMargin: 26

        D.DciIcon {
            sourceSize {
                width: 32
                height: 32
            }
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: 14
            name: Applet.iconName
        }

        ColumnLayout {
            spacing: 5
            Layout.alignment: Qt.AlignVCenter
            Loader {
                visible: active
                active: Applet.increaseVolume
                sourceComponent: scaleCom
                x: progressBar.visualPosition * 1.5 * progressBar.width
            }

            D.ProgressBar {
                id: progressBar
                Layout.preferredWidth: 200
                Layout.alignment: Qt.AlignVCenter
                from: 0
                value: Applet.volumeValue / (Applet.increaseVolume ? 1.5 : 1.0)
                to: 1
            }

            Loader {
                visible: active
                active: Applet.increaseVolume
                sourceComponent: scaleCom
                x: progressBar.visualPosition * 1.5 * progressBar.width
            }
        }

        Text {
            Layout.alignment: Qt.AlignVCenter
            font: D.DTK.fontManager.t4
            text: Number(Applet.volumeValue * 100).toFixed(0) + "%"
        }
    }

    Component {
        id: scaleCom
        Rectangle {
            height: 6
            width: 2
            color: "gray"
        }
    }
}
