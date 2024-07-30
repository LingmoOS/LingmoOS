/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick

import org.kde.lingmoui as LingmoUI

QtObject {
    property real gridUnit: LingmoUI.Units.gridUnit
    property real smallSpacing: LingmoUI.Units.smallSpacing
    property real largeSpacing: LingmoUI.Units.largeSpacing
    property real cornerRadius: LingmoUI.Units.cornerRadius

    property real smallIconSize: LingmoUI.Units.iconSizes.small

    property color highlightColor: LingmoUI.Theme.highlightColor
    property color backgroundColor: LingmoUI.Theme.backgroundColor

    LingmoUI.Theme.colorSet: LingmoUI.Theme.View
}
