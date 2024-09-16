// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk.settings 1.0 as Settings
import ".."

Settings.OptionDelegate {
    id: control

    LineEdit {
        id: impl
        Layout.alignment: Qt.AlignRight
        Layout.fillWidth: true

        text: Settings.SettingsOption.value || ""

        onEditingFinished: {
            Settings.SettingsOption.value = text
        }
    }
}
