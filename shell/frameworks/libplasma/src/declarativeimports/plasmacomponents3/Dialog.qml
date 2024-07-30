/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
//NOTE: importing PlasmaCore is necessary in order to make KSvg load the current Plasma Theme
import org.kde.plasma.core as PlasmaCore

T.Dialog {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentWidth > 0 ? contentWidth + leftPadding + rightPadding : 0)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentWidth > 0 ? contentHeight + topPadding + bottomPadding : 0)

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    topPadding: (background as KSvg.FrameSvgItem)?.margins.top ?? undefined
    leftPadding: (background as KSvg.FrameSvgItem)?.margins.left ?? undefined
    rightPadding: (background as KSvg.FrameSvgItem)?.margins.right ?? undefined
    bottomPadding: (background as KSvg.FrameSvgItem)?.margins.bottom ?? undefined

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            easing.type: Easing.InOutQuad
            duration: Kirigami.Units.longDuration
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InOutQuad
            duration: Kirigami.Units.longDuration
        }
    }

    contentItem: Item { }

    background: KSvg.FrameSvgItem {
        implicitWidth: Kirigami.Units.gridUnit * 12
        imagePath: "widgets/background"
    }

    footer: DialogButtonBox {
        position: DialogButtonBox.Footer
    }
}
