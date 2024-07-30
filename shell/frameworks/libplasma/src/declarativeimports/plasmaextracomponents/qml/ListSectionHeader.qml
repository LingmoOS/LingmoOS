/*
 *  SPDX-FileCopyrightText: 2019 Björn Feber <bfeber@protonmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents

/**
 * @brief A section delegate for the primitive ListView component.
 *
 * It's intended to make all listviews look coherent, mirroring the style
 * of the Kirigami version, but with the separator line being an SVG from the
 * Plasma theme rather than a simple line.
 *
 * Any additional content items will be positioned in a row at the trailing side
 * of this component.
 *
 *
 * Example usage:
 * @code
 * import QtQuick
 * import org.kde.plasma.components as PlasmaComponents
 * import org.kde.plasma.extras as PlasmaExtras
 *
 * ListView {
 *     section.delegate: PlasmaExtras.ListSectionHeader {
 *         label: section
 *
 *         PlasmaComponents.Button {
 *             text: "Button 1"
 *         }
 *         PlasmaComponents.Button {
 *             text: "Button 2"
 *         }
 *     }
 * }
 * @endcode
 */
PlasmaComponents.ItemDelegate {
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

    topPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

    contentItem: RowLayout {
        id: rowLayout
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            Layout.maximumWidth: rowLayout.width
            Layout.alignment: Qt.AlignVCenter

            opacity: 0.7
            level: 5
            type: Kirigami.Heading.Primary
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
