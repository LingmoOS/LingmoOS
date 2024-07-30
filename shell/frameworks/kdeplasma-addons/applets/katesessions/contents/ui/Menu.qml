/*
    SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    Based on the clipboard applet:
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PlasmaComponents3

PlasmaComponents3.ScrollView {
    id: menu
    property alias view: menuListView
    property alias model: menuListView.model
    signal itemSelected(string profileIdentifier)

    ListView {
        id: menuListView
        focus: true

        boundsBehavior: Flickable.StopAtBounds
        interactive: contentHeight > height
        highlight: PlasmaExtras.Highlight {
            anchors.bottomMargin: -listMargins.bottom
            y: 1
        }
        highlightMoveDuration: 0
        highlightResizeDuration: 0
        currentIndex: -1

        delegate: KateSessionsItemDelegate {
            width: menuListView.width

            onItemSelected: function(profileIdentifier) {
                menu.itemSelected(profileIdentifier)
            }
        }
    }
}
