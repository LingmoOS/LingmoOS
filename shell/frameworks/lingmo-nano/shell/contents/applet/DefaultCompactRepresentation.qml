/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI

Item {
    id: main

    Layout.minimumWidth: {
        switch (plasmoid.formFactor) {
        case LingmoCore.Types.Vertical:
            return 0;
        case LingmoCore.Types.Horizontal:
            return height;
        default:
            return LingmoUI.Units.gridUnit * 3;
        }
    }

    Layout.minimumHeight: {
        switch (plasmoid.formFactor) {
        case LingmoCore.Types.Vertical:
            return width;
        case LingmoCore.Types.Horizontal:
            return 0;
        default:
            return LingmoUI.Units.gridUnit * 3;
        }
    }

    LingmoUI.Icon {
        id: icon
        source: plasmoid.icon ? plasmoid.icon : "lingmo"
        active: mouseArea.containsMouse
        LingmoUI.Theme.colorSet: LingmoUI.Theme.ComplementaryColorGroup
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        id: mouseArea

        property bool wasExpanded: false

        anchors.fill: parent
        hoverEnabled: true
        onPressed: wasExpanded = plasmoid.expanded
        onClicked: plasmoid.expanded = !wasExpanded
    }
}
