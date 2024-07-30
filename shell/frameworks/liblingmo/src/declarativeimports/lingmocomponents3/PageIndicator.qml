/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.PageIndicator {
    id: control

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    padding: LingmoUI.Units.smallSpacing
    spacing: LingmoUI.Units.smallSpacing

    delegate: Rectangle {
        implicitWidth: LingmoUI.Units.largeSpacing
        implicitHeight: implicitWidth

        radius: width
        color: LingmoUI.Theme.textColor

        opacity: index === currentIndex ? 0.9 : pressed ? 0.7 : 0.5
        Behavior on opacity {
            enabled: LingmoUI.Units.longDuration > 0
            OpacityAnimator {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    contentItem: Row {
        spacing: control.spacing

        Repeater {
            model: control.count
            delegate: control.delegate
        }
    }
}
