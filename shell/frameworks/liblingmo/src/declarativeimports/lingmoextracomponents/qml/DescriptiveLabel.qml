/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.components

/**
 * This is a descriptive label which uses the lingmo theme. The characteristics of
 * the text will be automatically set according to the lingmo theme. Use this
 * components for less important additional data to show in a user interface.
 *
 * Example usage:
 * @code
 * import org.kde.lingmo.extras as LingmoExtras
 * [...]
 * Column {
 *     LingmoComponents.Label { text: "Firefox" }
 *     LingmoExtras.DescriptiveLabel { text: "Web Browser"}
 *   [...]
 * }
 * @endcode
 *
 * See LingmoComponents Label and primitive QML Text element API for additional
 * properties, methods and signals.
 *
 * @inherits org::kde::lingmo::components::Label
 */
Label {
    id: root

    /*
     * If a user can interact with this item, for example in a ListView delegate, this
     * property should be set to true when the label is being interacted with.
     * The default is false.
     */
    property bool active: false

    opacity: active ? 0.8 : 0.6
}
