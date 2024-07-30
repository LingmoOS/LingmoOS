/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.Slider {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitHandleHeight + topPadding + bottomPadding)

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false

    hoverEnabled: Qt.styleHints.useHoverEffects

    padding: LingmoUI.Settings.tabletMode ? LingmoUI.Units.mediumSpacing : 0

    handle: Impl.SliderHandle {
        control: control
    }

    /* NOTE: Qt QQC2 styles use the background property for the groove
     * and fill. It doesn't really make sense when you look at the way
     * backgrounds typically work. Originally, there was a `track` property,
     * but it got replaced by `background` in 2016.
     *
     * Neither of the background or contentItem properties are perfect
     * for use as the property to hold the groove or the fill.
     * The background seems like it could be OK for the groove and the
     * contentItem could be OK for the fill.
     *
     * However, we're keeping them both in the background like Qt to avoid
     * any potiential compatiblity issues where app devs assume that the
     * contentItem will always be unused by the style.
     */

    // groove
    background: Impl.SliderGroove {
        control: control
        startPosition: 0
        endPosition: control.position
    }
}
