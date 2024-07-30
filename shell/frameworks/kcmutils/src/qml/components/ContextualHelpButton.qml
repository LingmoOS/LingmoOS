/*
   SPDX-FileCopyrightText: 2020 Felix Ernst <fe.a.ernst@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.7 as QQC2
import QtQuick.Window 2.15
import org.kde.lingmoui 2.3 as LingmoUI

@Deprecated { reason: "Use the version in LingmoUI instead!" }
QQC2.Button {
    id: root

    icon.name: "help-contextual"
    flat: true
    property alias toolTipText: toolTip.text
    property var toolTipVisible: false

    onReleased: {
        toolTipVisible ? toolTip.delay = LingmoUI.Units.toolTipDelay : toolTip.delay = 0;
        toolTipVisible = !toolTipVisible;
    }
    onActiveFocusChanged: {
        toolTip.delay = LingmoUI.Units.toolTipDelay;
        toolTipVisible = false;
    }
    Layout.maximumHeight: parent.height
    QQC2.ToolTip {
        id: toolTip
        implicitWidth: Math.min(21 * LingmoUI.Units.gridUnit, root.Window.width) // Wikipedia says anything between 45 and 75 characters per line is acceptable. 21 * LingmoUI.Units.gridUnit feels right.
        visible: parent.hovered || parent.toolTipVisible
        onVisibleChanged: {
            if (!visible && parent.toolTipVisible) {
                parent.toolTipVisible = false;
                delay = LingmoUI.Units.toolTipDelay;
            }
        }
        timeout: -1
    }
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.WhatsThisCursor
        acceptedButtons: Qt.NoButton
    }
    Accessible.name: i18ndc("kcmutils6", "@action:button", "Show Contextual Help")
}

