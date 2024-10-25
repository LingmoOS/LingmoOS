/*
 *  SPDX-FileCopyrightText: 2019 Björn Feber <bfeber@protonmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI
import org.kde.ksvg as KSvg
import org.kde.lingmo.components as LingmoComponents

/**
 * @brief A section delegate for the primitive ListView component.
 *
 * It's intended to make all listviews look coherent, mirroring the style
 * of the LingmoUI version, but with the separator line being an SVG from the
 * Lingmo theme rather than a simple line.
 *
 * Any additional content items will be positioned in a row at the trailing side
 * of this component.
 *
 *
 * Example usage:
 * @code
 * import QtQuick
 * import org.kde.lingmo.components as LingmoComponents
 * import org.kde.lingmo.extras as LingmoExtras
 *
 * ListView {
 *     section.delegate: LingmoExtras.ListSectionHeader {
 *         label: section
 *
 *         LingmoComponents.Button {
 *             text: "Button 1"
 *         }
 *         LingmoComponents.Button {
 *             text: "Button 2"
 *         }
 *     }
 * }
 * @endcode
 */
LingmoComponents.ItemDelegate {
    id: listSection

    /**
     * @brief This property sets the text of the ListView's section header.
     * @property string label
     */
    property alias label: listSection.text

    default property alias _contents: rowLayout.data

    hoverEnabled: false

    activeFocusOnTab: false

    // we do not need a background
    background: null

    topPadding: LingmoUI.Units.largeSpacing + LingmoUI.Units.smallSpacing

    contentItem: RowLayout {
        id: rowLayout
        spacing: LingmoUI.Units.largeSpacing

        LingmoUI.Heading {
            Layout.maximumWidth: rowLayout.width
            Layout.alignment: Qt.AlignVCenter

            opacity: 0.7
            level: 5
            type: LingmoUI.Heading.Primary
            text: listSection.text
            elide: Text.ElideRight

            // we override the Primary type's font weight (DemiBold) for Bold for contrast with small text
            font.weight: Font.Bold

            Accessible.ignored: true
        }

        KSvg.SvgItem {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            imagePath: "widgets/line"
            elementId: "horizontal-line"

            Accessible.ignored: true
        }
    }
}
