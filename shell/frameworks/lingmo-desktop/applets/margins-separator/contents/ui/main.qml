/*
    SPDX-FileCopyrightText: 2020 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.4
import QtQuick.Layouts 1.0
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.ksvg 1.0 as KSvg


PlasmoidItem {
    id: root

    readonly property bool isVertical: Plasmoid.formFactor === LingmoCore.Types.Vertical

    Layout.minimumWidth: Plasmoid.containment.corona.editMode && !isVertical ? LingmoUI.Units.gridUnit : 1
    Layout.preferredWidth: Layout.minimumWidth
    Layout.maximumWidth:   Layout.minimumWidth

    Layout.minimumHeight: Plasmoid.containment.corona.editMode && isVertical ? LingmoUI.Units.gridUnit : Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight
    Layout.maximumHeight: Layout.minimumHeight

    Plasmoid.constraintHints: Plasmoid.MarginAreasSeparator
    preferredRepresentation: fullRepresentation

    Loader {
        anchors.centerIn: parent
        active: Plasmoid.containment.corona.editMode
        sourceComponent: KSvg.SvgItem {
            height: root.isVertical ? 1 : Math.round(root.height / 2)
            width: root.isVertical ? Math.round(root.width / 2) : 1
            imagePath: "widgets/line"
            elementId: root.isVertical ? "vertical-line" : "horizontal-line"
        }
    }
}
