/*
 *  SPDX-FileCopyrightText: 2020 Sora Steenvoort <sora@dillbox.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.0
import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "General")
         icon: "plasma"
         source: "ConfigGeneral.qml"
    }

    ConfigCategory {
         name: i18nc("@title", "Appearance")
         icon: "preferences-desktop-color"
         source: "ConfigAppearance.qml"
    }
}
