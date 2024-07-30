/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick

pragma Singleton;

QtObject {
    property Loader kirigamiThemeLoader: Loader {
        source: "KirigamiTheme.qml"
    }

    property real gridUnit: kirigamiThemeLoader.item?.gridUnit ?? 20
    property real smallSpacing: kirigamiThemeLoader.item?.smallSpacing ?? 5
    property real largeSpacing: kirigamiThemeLoader.item?.largeSpacing ?? 10
    property real cornerRadius: kirigamiThemeLoader.item?.cornerRadius ?? 5

    property real smallIconSize: kirigamiThemeLoader.item?.smallIconSize ?? 16

    property color highlightColor: kirigamiThemeLoader.item?.highlightColor ?? "blue"
    property color backgroundColor: kirigamiThemeLoader.item?.backgroundColor ?? "white"
}
