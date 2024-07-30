/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.Dialog {
    id: control

    z: LingmoUI.OverlayZStacking.z

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding,
                            implicitHeaderWidth,
                            implicitFooterWidth)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding
                             + (implicitHeaderHeight > 0 ? implicitHeaderHeight + spacing : 0)
                             + (implicitFooterHeight > 0 ? implicitFooterHeight + spacing : 0))

    padding: LingmoUI.Units.gridUnit

    // black background, fades in and out
    QQC2.Overlay.modal: Rectangle {
        color: Qt.rgba(0, 0, 0, 0.3)

        // the opacity of the item is changed internally by QQuickPopup on open/close
        Behavior on opacity {
            enabled: LingmoUI.Units.longDuration > 0
            OpacityAnimator {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

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

    contentItem: Item {}

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
}
