/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.configuration
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore

ConfigModel {
    ConfigCategory {
        name: i18nc("@title", "Weather Station")
        icon: "services"
        source: "config/ConfigWeatherStation.qml"
    }

    ConfigCategory {
        name: i18nc("@title", "Appearance")
        icon: "preferences-desktop-color"
        source: "config/ConfigAppearance.qml"
        // This category's settings only apply to the CompactRepresentation in the panel
        visible: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.BottomEdge,
                  PlasmaCore.Types.LeftEdge, PlasmaCore.Types.RightEdge,
                 ].includes(Plasmoid.location)
    }

    ConfigCategory {
        name: i18nc("@title", "Units")
        icon: "preferences-other"
        source: "config/ConfigUnits.qml"
    }
}
