/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMutils

/**
 * This component is intended to be used as the root item for KCMs that are based upon
 * a grid view of thumbnails, such as the theme or wallpaper selectors.
 * It contains a GridView as its main item.
 * It is possible to specify a header and footer component.
 * @code
 * import org.kde.kcmutils as KCMutils
 *
 * KCMutils.GridViewKCM {
 *     header: Item { }
 *     view.model: kcm.model
 *     view.delegate: KCMutils.GridDelegate { }
 *     footer: Item { }
 * }
 * @endcode
 */
KCMutils.AbstractKCM {
    id: root

    /**
     * view: GridView
     * Exposes the internal GridView: in order to set a model or a delegate to it,
     * use the following code:
     * @code
     * import org.kde.kcmutils as KCMutils
     *
     * KCMutils.GridViewKCM {
     *     view.model: kcm.model
     *     view.delegate: KCMutils.GridDelegate { }
     * }
     * @endcode
     */
    property alias view: scroll.view

    /**
     * framedView: bool
     * Whether to draw a frame around the KCM's inner scrollable grid view.
     * Default: false
     */
    framedView: false

    implicitWidth: {
        let width = 0;

        // Show three columns at once, every column occupies implicitCellWidth + Units.gridUnit
        width += 3 * (view.implicitCellWidth + Kirigami.Units.gridUnit);

        const scrollBar = scroll.QQC2.ScrollBar.vertical;
        width += scrollBar.width + scrollBar.leftPadding + scrollBar.rightPadding;

        width += scroll.leftPadding + scroll.rightPadding
        width += root.leftPadding + root.rightPadding;

        return width;
    }
    implicitHeight: view.implicitCellHeight * 3 + (header ? header.height : 0) + (footer ? footer.height : 0) + Kirigami.Units.gridUnit

    flickable: scroll.view

    KCMutils.GridView {
        id: scroll
        anchors.fill: parent
        framedView: root.framedView
    }
}
