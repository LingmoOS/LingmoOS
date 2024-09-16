// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.settings 1.0 as Settings

Settings.SettingsContainer {
    navigationTitle: Rectangle {}
    contentTitle: Rectangle {}
    contentBackground: Rectangle {}

    groups: [
        Settings.SettingsGroup {
            key: "key-group1"
            name: "group1"
            Settings.SettingsOption {
                key: "key-option1"
                name: "option1"
                Rectangle {}
            }
            Settings.SettingsOption {
                key: "key-option2"
                name: "option2"
                Rectangle {}
            }
            children: [
                Settings.SettingsGroup {}
            ]
        },
        Settings.SettingsGroup {
            key: "key-group2"
            name: "group2"
            Settings.SettingsOption {
                key: "key-option1"
                name: "option1"
                Rectangle {}
            }
            background: Rectangle {}
        }
    ]
}
