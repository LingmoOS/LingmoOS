// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk.settings 1.0 as Settings
import ".."

Settings.OptionDelegate {
    id: control

    property alias impl: impl
    property string valueRole
    property alias model: impl.model

    ComboBox {
        id: impl
        Layout.alignment: Qt.AlignRight
        Layout.fillWidth: true

        currentIndex: {
            var index = -1
            for(var i = 0; i < model.length; i++) {
                var item
                if (valueRole === "") {
                    item = model[i]
                } else {
                    item = model[i][valueRole]
                }
                if (item === Settings.SettingsOption.value) {
                    index = i
                    break
                }
            }
            return index
        }
        onActivated: {
            if (valueRole === "") {
                Settings.SettingsOption.value = control.model[currentIndex]
            } else {
                Settings.SettingsOption.value = control.model[currentIndex][valueRole]
            }
        }
    }
}
