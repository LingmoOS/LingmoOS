/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.plasmoid

PlasmoidItem {
    id: root

    Plasmoid.title: "Representations Example"

    switchWidth: LingmoUI.Units.gridUnit * 10
    switchHeight: LingmoUI.Units.gridUnit * 10

    compactRepresentation: MouseArea {
        property bool wasExpanded

        Accessible.name: Plasmoid.title
        Accessible.role: Accessible.Button

        Layout.minimumWidth: LingmoUI.Units.gridUnit * 3
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 3

        onPressed: wasExpanded = root.expanded
        onClicked: root.expanded = !wasExpanded

        LingmoComponents.Label {
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            text: i18n("Click me")
        }
    }

    fullRepresentation: LingmoExtras.Representation {
        Layout.preferredWidth: LingmoUI.Units.gridUnit * 20
        Layout.preferredHeight: LingmoUI.Units.gridUnit * 20

        Layout.minimumWidth: root.switchWidth
        Layout.minimumHeight: root.switchHeight

        header: LingmoExtras.BasicPlasmoidHeading {}
        contentItem: LingmoComponents.ScrollView {
            contentWidth: availableWidth
            LingmoExtras.Heading {
                anchors.fill: parent
                topPadding: LingmoUI.Units.gridUnit * 2
                bottomPadding: LingmoUI.Units.gridUnit * 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                text: i18n("Hello world")
            }
        }
    }
}
