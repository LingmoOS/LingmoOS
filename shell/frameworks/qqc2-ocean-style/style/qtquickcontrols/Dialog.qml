/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.Dialog {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            Math.ceil(contentWidth) + leftPadding + rightPadding,
                            implicitHeaderWidth,
                            implicitFooterWidth)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             Math.ceil(contentHeight) + topPadding + bottomPadding
                             + (implicitHeaderHeight > 0 ? implicitHeaderHeight + spacing : 0)
                             + (implicitFooterHeight > 0 ? implicitFooterHeight + spacing : 0))

    padding: LingmoUI.Units.gridUnit

//     anchors.centerIn: T.Overlay.overlay

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            easing.type: Easing.InOutQuad
            duration: LingmoUI.Units.longDuration
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InOutQuad
            duration: LingmoUI.Units.longDuration
        }
    }

    background: LingmoUI.ShadowedRectangle {
        LingmoUI.Theme.colorSet: LingmoUI.Theme.View
        LingmoUI.Theme.inherit: false

        color: LingmoUI.Theme.backgroundColor
        radius: LingmoUI.Units.cornerRadius

        shadow {
            size: radius * 2
            color: Qt.rgba(0, 0, 0, 0.3)
            yOffset: 1
        }

        border {
            width: 1
            color: LingmoUI.ColorUtils.linearInterpolation(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, LingmoUI.Theme.frameContrast);
        }
    }

    header: LingmoUI.Heading {
        text: control.title
        level: 2
        visible: control.title
        elide: Label.ElideRight
        padding: LingmoUI.Units.gridUnit
        bottomPadding: 0
    }

    footer: DialogButtonBox {
        visible: count > 0
    }

    T.Overlay.modal: Impl.OverlayModalBackground {}
    T.Overlay.modeless: Impl.OverlayDimBackground {}
}
