/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI

Row {
    height: 200
    width: 400
    LingmoUI.Icon {
        source: "lingmo"
        LingmoCore.ToolTipArea {
            anchors.fill: parent
            mainText: "Tooltip Title"
            subText: "Some explanation."
            icon: "lingmo"
        }
    }

    LingmoUI.Icon {
        source: "ark"
        LingmoCore.ToolTipArea {
            anchors.fill: parent
            mainText: "Tooltip title"
            subText: "No icon!"
        }
    }
}

