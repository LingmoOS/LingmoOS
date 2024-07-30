/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick

import org.kde.kirigami as Kirigami

QtObject {
    property real gridUnit: Kirigami.Units.gridUnit
    property real smallSpacing: Kirigami.Units.smallSpacing
    property real largeSpacing: Kirigami.Units.largeSpacing
    property real cornerRadius: Kirigami.Units.cornerRadius

    property real smallIconSize: Kirigami.Units.iconSizes.small

    property color highlightColor: Kirigami.Theme.highlightColor
    property color backgroundColor: Kirigami.Theme.backgroundColor

    Kirigami.Theme.colorSet: Kirigami.Theme.View
}
