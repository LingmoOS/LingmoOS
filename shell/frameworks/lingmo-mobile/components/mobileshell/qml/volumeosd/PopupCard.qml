/*
 *  SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtQuick.Window

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmo.components 3.0 as LingmoComponents

// capture presses on the audio applet so it doesn't close the overlay
Controls.Control {
    id: content
    implicitWidth: Math.min(LingmoUI.Units.gridUnit * 20, parent.width - LingmoUI.Units.gridUnit * 2)
    padding: LingmoUI.Units.smallSpacing * 2
    background: KSvg.FrameSvgItem {
        imagePath: "widgets/background"
        anchors.margins: -LingmoUI.Units.smallSpacing * 2
        anchors.fill: parent
    }
}
