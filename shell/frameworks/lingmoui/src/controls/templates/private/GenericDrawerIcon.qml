/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

Item {
    width: height
    height: LingmoUI.Units.iconSizes.smallMedium
    property LingmoUI.OverlayDrawer drawer
    property color color: LingmoUI.Theme.textColor
    opacity: 0.8
    layer.enabled: true

    LingmoUI.Icon {
        selected: drawer.handle.pressed
        opacity: 1 - drawer.position
        anchors.fill: parent
        source: drawer.handleClosedIcon.name ? drawer.handleClosedIcon.name : drawer.handleClosedIcon.source
        color: drawer.handleClosedIcon.color
    }
    LingmoUI.Icon {
        selected: drawer.handle.pressed
        opacity: drawer.position
        anchors.fill: parent
        source: drawer.handleOpenIcon.name ? drawer.handleOpenIcon.name : drawer.handleOpenIcon.source
        color: drawer.handleOpenIcon.color
    }
}

