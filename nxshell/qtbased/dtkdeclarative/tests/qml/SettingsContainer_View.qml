// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.settings 1.0 as Settings

Settings.SettingsContainer {
    navigationTitle: Rectangle {
        color: "red";
        property string name: Settings.SettingsGroup.name
        property int level: Settings.SettingsGroup.level
    }
    contentTitle: Rectangle {
        color: "blue";
        property string name: Settings.SettingsGroup.name
        property int level: Settings.SettingsGroup.level
    }
    contentBackground: Rectangle {
        color: "#FFFFF0";
        property string name: Settings.SettingsGroup.name
    }

    groups: [
        Settings.SettingsGroup {
            key: "key-group1"
            name: "group1"
            Settings.SettingsOption {
                key: "key-option1"
                name: "option1"
                Rectangle {
                    color: "#FFFFF1"
                    property string name: Settings.SettingsOption.name
                }
            }
            Settings.SettingsOption {
                key: "key-option2"
                name: "option2"
                Rectangle {
                    property string name: Settings.SettingsOption.name
                }
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
                Rectangle {
                    color: "#FFFFF2";
                    property string name: Settings.SettingsOption.name
                }
            }
        }
    ]
}
