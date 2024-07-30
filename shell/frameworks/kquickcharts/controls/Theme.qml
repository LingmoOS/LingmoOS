/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick

pragma Singleton;

QtObject {
    property Loader lingmouiThemeLoader: Loader {
        source: "LingmoUITheme.qml"
    }

    property real gridUnit: lingmouiThemeLoader.item?.gridUnit ?? 20
    property real smallSpacing: lingmouiThemeLoader.item?.smallSpacing ?? 5
    property real largeSpacing: lingmouiThemeLoader.item?.largeSpacing ?? 10
    property real cornerRadius: lingmouiThemeLoader.item?.cornerRadius ?? 5

    property real smallIconSize: lingmouiThemeLoader.item?.smallIconSize ?? 16

    property color highlightColor: lingmouiThemeLoader.item?.highlightColor ?? "blue"
    property color backgroundColor: lingmouiThemeLoader.item?.backgroundColor ?? "white"
}
