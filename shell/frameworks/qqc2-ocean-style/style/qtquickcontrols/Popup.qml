/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL
 */


import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.Popup {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    padding: Impl.Units.veryLargeSpacing
//     clip: true

    enter: Transition {
        OpacityAnimator {
            //property: "opacity"
            from: 0
            to: 1
            easing.type: Easing.InOutQuad
            duration: LingmoUI.Units.shortDuration
        }
    }

    exit: Transition {
        OpacityAnimator {
            //property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InOutQuad
            duration: LingmoUI.Units.shortDuration
        }
    }

    background: Rectangle {
        radius: Impl.Units.smallRadius
//         implicitHeight: Impl.Units.smallControlHeight
        //implicitWidth: LingmoUI.Units.gridUnit * 12
        color: LingmoUI.Theme.backgroundColor

        border {
            color: Impl.Theme.separatorColor()
            width: control.dim ? 0 : Impl.Units.smallBorder
        }

        Impl.MediumShadow {
            radius: parent.radius
        }
    }

    T.Overlay.modal: Impl.OverlayModalBackground {}
    T.Overlay.modeless: Impl.OverlayDimBackground {}
}
