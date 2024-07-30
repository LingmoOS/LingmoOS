/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.lingmoui 2.15 as LingmoUI
import org.kde.lingmo.configuration 2.0
import org.kde.lingmo.plasmoid 2.0

AppletConfiguration {
    id: root
    isContainment: true
    Layout.minimumWidth: LingmoUI.Units.gridUnit * 35
    Layout.minimumHeight: LingmoUI.Units.gridUnit * 30
    Layout.preferredWidth: LingmoUI.Units.gridUnit * 32
    Layout.preferredHeight: LingmoUI.Units.gridUnit * 36

//BEGIN model
    globalConfigModel: globalContainmentConfigModel

    ConfigModel {
        id: globalContainmentConfigModel
        ConfigCategory {
            name: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Wallpaper")
            icon: "preferences-desktop-wallpaper"
            source: "ConfigurationContainmentAppearance.qml"
            includeMargins: false
        }
        ConfigCategory {
            name: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Mouse Actions")
            icon: "preferences-desktop-mouse"
            source: "ConfigurationContainmentActions.qml"
        }
    }
//END model

}
