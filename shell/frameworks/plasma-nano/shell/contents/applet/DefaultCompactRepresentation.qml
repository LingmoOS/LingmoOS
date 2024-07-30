/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami

Item {
    id: main

    Layout.minimumWidth: {
        switch (plasmoid.formFactor) {
        case PlasmaCore.Types.Vertical:
            return 0;
        case PlasmaCore.Types.Horizontal:
            return height;
        default:
            return Kirigami.Units.gridUnit * 3;
        }
    }

    Layout.minimumHeight: {
        switch (plasmoid.formFactor) {
        case PlasmaCore.Types.Vertical:
            return width;
        case PlasmaCore.Types.Horizontal:
            return 0;
        default:
            return Kirigami.Units.gridUnit * 3;
        }
    }

    Kirigami.Icon {
        id: icon
        source: plasmoid.icon ? plasmoid.icon : "plasma"
        active: mouseArea.containsMouse
        Kirigami.Theme.colorSet: Kirigami.Theme.ComplementaryColorGroup
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
