// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.7
import Qt.labs.platform 1.1
import org.deepin.dtk 1.0
import org.deepin.tweak 1.0

ColumnLayout {
    id: frame
    spacing: 10
    property string displayName: qsTr("Title height")
    property string description: qsTr("Change window title height")
    property string version: "1.0.0"
    property string author: "justforlxz"
    property string icon: "icon.png"

    function getConfigFilePath(theme) {
        const paths = StandardPaths.standardLocations(StandardPaths.GenericDataLocation)
        if (paths.length === 0) {
            return;
        }
        return paths[0].slice('files://'.length-1) + '/deepin/themes/deepin/'+ theme + '/titlebar.ini';
    }
    function loadConfig() {
        const path = getConfigFilePath('light')
        const settings = Tweak.newSettings(path);
        settings.beginGroup('Active');
        const height = settings.value('height');
        if (height !== undefined) {
            control.value = height
        }
    }

    function setHeight(value) {
        const setTitleBar = function(theme, value) {
            const path = getConfigFilePath(theme)
            const settings = Tweak.newSettings(path);
            settings.beginGroup('Active');
            settings.setValue('height', value);
            settings.endGroup();
            settings.beginGroup('Inactive');
            settings.setValue('height', value);
        }
        setTitleBar('light', value);
        setTitleBar('dark', value);

        // restart wm
        // TODO: use dbus but wait for v23
        const wm = Tweak.newLauncher();
        wm.program('/usr/bin/kwin_no_scale');
        wm.arguments(['--replace']);
        wm.startDetached();
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
            to: 9999
            value: 40
            editable: true
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true

            validator: IntValidator {
                locale: control.locale.name
                bottom: Math.min(control.from, control.to)
                top: Math.max(control.from, control.to)
            }
        }
        Button {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            text: qsTr("Reset to default")
            onClicked: {
                setHeight(40)
                loadConfig()
            }
        }
    }

    Item {
        Layout.fillHeight: true
    }

    Button {
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        Layout.bottomMargin: 10
        id: accept
        text: qsTr("Apply")
        onClicked: setHeight(control.value)
    }

    Component.onCompleted: loadConfig()
}
