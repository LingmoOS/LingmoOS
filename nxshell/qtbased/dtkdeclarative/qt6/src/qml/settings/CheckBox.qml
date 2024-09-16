// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.settings 1.0 as Settings
import org.deepin.dtk 1.0

Settings.OptionDelegate {
    id: control
    leftVisible: false

    CheckBox {
        Layout.fillWidth: true

        text: Settings.SettingsOption.name
        checked: Settings.SettingsOption.value || false
        onCheckedChanged: {
            Settings.SettingsOption.value = checked
        }
    }
}
