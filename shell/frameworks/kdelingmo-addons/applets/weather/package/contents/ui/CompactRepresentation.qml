/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.workspace.components as WorkspaceComponents

Loader {
    id: compactRoot

    property var generalModel
    property var observationModel

    readonly property bool vertical: (Plasmoid.formFactor == LingmoCore.Types.Vertical)
    readonly property bool showTemperature: Plasmoid.configuration.showTemperatureInCompactMode
    readonly property bool useBadge: Plasmoid.configuration.showTemperatureInBadge || Plasmoid.needsToBeSquare

    sourceComponent: (showTemperature && !useBadge) ? iconAndTextComponent : iconComponent
    Layout.fillWidth: compactRoot.vertical
    Layout.fillHeight: !compactRoot.vertical
    Layout.minimumWidth: item.Layout.minimumWidth
    Layout.minimumHeight: item.Layout.minimumHeight

    MouseArea {
        id: compactMouseArea
        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            root.expanded = !root.expanded;
        }
    }

    Component {
        id: iconComponent

        LingmoUI.Icon {
            readonly property int minIconSize: Math.max((compactRoot.vertical ? compactRoot.width : compactRoot.height), LingmoUI.Units.iconSizes.small)

            source: Plasmoid.icon
            active: compactMouseArea.containsMouse
            // reset implicit size, so layout in free dimension does not stop at the default one
            implicitWidth: LingmoUI.Units.iconSizes.small
            implicitHeight: LingmoUI.Units.iconSizes.small
            Layout.minimumWidth: compactRoot.vertical ? LingmoUI.Units.iconSizes.small : minIconSize
            Layout.minimumHeight: compactRoot.vertical ? minIconSize : LingmoUI.Units.iconSizes.small

            WorkspaceComponents.BadgeOverlay {
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                visible: showTemperature && useBadge && text.length > 0

                text: observationModel.temperature
                icon: parent
            }
        }
    }

    Component {
        id: iconAndTextComponent

        IconAndTextItem {
            vertical: compactRoot.vertical
            iconSource: Plasmoid.icon
            active: compactMouseArea.containsMouse
            text: observationModel.temperature
        }
    }
}
