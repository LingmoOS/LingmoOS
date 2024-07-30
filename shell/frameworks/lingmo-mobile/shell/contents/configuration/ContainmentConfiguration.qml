// SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
// SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.12
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2
import QtQuick.Controls 2.15 as Controls

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.configuration 2.0
import org.kde.ksvg 1.0 as KSvg

AppletConfiguration {
    id: root
    isContainment: true
    loadApp: true

    readonly property bool horizontal: root.width > root.height

    onAppLoaded: {
        app.width = root.width < root.height ? root.width : Math.min(root.width, Math.max(app.implicitWidth, LingmoUI.Units.gridUnit * 45));
        app.height = Math.min(root.height, Math.max(app.implicitHeight, LingmoUI.Units.gridUnit * 29));
    }

//BEGIN model
    globalConfigModel: globalContainmentConfigModel

    ConfigModel {
        id: globalContainmentConfigModel
        ConfigCategory {
            name: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Wallpaper")
            icon: "preferences-desktop-wallpaper"
            source: "ConfigurationContainmentAppearance.qml"
        }
    }
//END model
}
