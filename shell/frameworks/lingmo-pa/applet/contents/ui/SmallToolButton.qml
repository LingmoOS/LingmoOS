/*
    SPDX-FileCopyrightText: 2017 Chris Holland <zrenfire@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents3

LingmoComponents3.ToolButton {
    id: smallToolButton
    readonly property int size: Math.ceil(LingmoUI.Units.iconSizes.small + LingmoUI.Units.smallSpacing * 2)
    implicitWidth: size
    implicitHeight: size

    display: LingmoComponents3.AbstractButton.IconOnly
}
