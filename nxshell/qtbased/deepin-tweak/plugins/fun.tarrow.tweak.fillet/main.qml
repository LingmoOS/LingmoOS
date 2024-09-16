// SPDX-FileCopyrightText: 2023 ice <tonimayloneya@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import Nemo.DBus 2.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.7
import org.deepin.dtk 1.0
import org.deepin.tweak 1.0

ColumnLayout {
    id: root

    property string displayName: qsTr("Fillet size")
    property string description: qsTr("Adjust window fillet size")
    property string version: "0.0.2"
    property string author: "ice"
    property string icon: "icon.svg"
    property int filletSize: 18
    property string appearanceService: "org.deepin.dde.Appearance1"
    property string appearancePath: "/org/deepin/dde/Appearance1"
    property string appearanceInterface: "org.deepin.dde.Appearance1"

    function getFilletSize() {
        filletSize = appearanceInter.getProperty("WindowRadius");
    }

    function setFilletSize(_size) {
        appearanceInter.setProperty("WindowRadius", _size);
    }

    Component.onCompleted: getFilletSize()

    DBusInterface {
        id: appearanceInter

        service: appearanceService
        iface: appearanceInterface
        path: appearancePath
    }

    Label {
        Layout.topMargin: 10
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        text: qsTr("Fillet size")
        font.bold: true
        font.pixelSize: 15
    }

    RowLayout {
        spacing: 10
        Layout.fillWidth: true
        Layout.topMargin: 10
        Layout.leftMargin: 10
        Layout.rightMargin: 10

        SpinBox {
            id: control

            from: 0
            to: 999
            value: filletSize
            editable: true
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        Button {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.preferredWidth: 118
            text: qsTr("Reset to default")
            onClicked: {
                setFilletSize(18);
            }
        }

    }

    Item {
        Layout.fillHeight: true
    }

    Button {
        id: accept

        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        Layout.bottomMargin: 10
        text: qsTr("Apply")
        onClicked: {
            setFilletSize(control.value);
        }
    }

}
