/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QtControls
import org.kde.lingmoui as LingmoUI

Column {
    id: root
    property alias cfg_Speed: slider.value

    Row {
        spacing: LingmoUI.Units.largeSpacing

        QtControls.Label {
            width: formAlignment - LingmoUI.Units.largeSpacing * 2
            horizontalAlignment: Text.AlignRight
            text: "Speed:"
        }
        QtControls.Slider {
            id: slider
            from: .2
            to: 1.5
        }
    }
}
