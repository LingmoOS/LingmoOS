/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12

import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmo.private.containmentlayoutmanager 1.0 as ContainmentLayoutManager
import org.kde.lingmoui 2.20 as LingmoUI

ContainmentLayoutManager.ItemContainer {
    enabled: false
    KSvg.FrameSvgItem {
        anchors.fill:parent
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0.5
    }
    Behavior on opacity {
        NumberAnimation {
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
}
