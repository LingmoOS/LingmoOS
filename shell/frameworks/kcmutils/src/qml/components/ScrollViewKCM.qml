/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kcmutils as KCMUtils

/**
 * This component is intended to be used as the root item for KCMs that are based upon a list view or another vertical flickable.
 * It contains a ScrollView as its main item.
 * It is possible to specify a header and footer component.
 * @code
 * import org.kde.kcmutils as KCMUtils
 *
 * KCMUtils.ScrollViewKCM {
 *     header: Item { }
 *     view: ListView { }
 *     footer: Item { }
 * }
 * @endcode
 */
KCMUtils.AbstractKCM {
    id: root

    /**
     * view: ScrollView
     * Exposes the internal flickable
     */
    property alias view: scroll.view

    /**
     * framedView: bool
     * Whether to draw a frame around the KCM's inner scrollable list view.
     * Default: false
     *
     * @since 5.90
     */
    framedView: false

    onViewChanged: {
        if (view) {
            // Deliberately don't take separators into account, because those are opaque anyway
            view.clip = Qt.binding(() => __headerContentVisible() || __footerContentVisible());
        }
    }

    KCMUtils.ScrollView {
        id: scroll
        anchors.fill: parent
        framedView: root.framedView
    }
}
