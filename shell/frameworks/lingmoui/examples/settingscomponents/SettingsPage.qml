/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import org.kde.lingmouiaddons.settings as AddonsSettings

AddonsSettings.CategorizedSettings {
    actions: [
        AddonsSettings.SettingAction {
            text: qsTr("General")
            actionName: qsTr("General")
            icon.name: "wayland"
            page: Qt.resolvedUrl("./GeneralSettingsPage.qml")
        }
    ]
}
