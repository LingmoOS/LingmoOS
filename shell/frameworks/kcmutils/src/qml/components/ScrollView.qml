/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * A ScrollView containing a GridView, with the default behavior about
 * sizing and background as recommended by the user interface guidelines
 * For most KControl modules, it's recommended to use instead the GridViewKCM
 * component as the root element of your module.
 * @code
 * import org.kde.kcmutils as KCMUtils
 *
 * KCMUtils.ScrollView {
 *     view: ListView { }
 * }
 * @endcode
 * @see GridViewKCM
 */
QQC2.ScrollView {
    id: scroll

    /**
     * view: GridView
     * Exposes the internal flickable
     */
    property Flickable view
    property bool framedView: true

    contentItem: view
    onViewChanged: {
        view.parent = scroll;
        if (!view.KeyNavigation.up) {
            view.KeyNavigation.up = Qt.binding(() => root.globalToolBarItem);
        }
    }

    activeFocusOnTab: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Component.onCompleted: {
        if (background) {
            background.visible = Qt.binding(() => framedView);
        }
    }

    QQC2.ScrollBar.horizontal.visible: false
}
