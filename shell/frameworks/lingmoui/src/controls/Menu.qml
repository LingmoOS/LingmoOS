/*
 * Copyright (C) 2024 Lingmo OS Team <team@lingmo.org>
 *
 * Author: Lingmo OS Team <team@lingmo.org>
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Window 2.12
import org.kde.lingmoui as LingmoUI
import Qt5Compat.GraphicalEffects

T.Menu
{
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    margins: 0
    verticalPadding: LingmoUI.Units.smallSpacing
    spacing: LingmoUI.Units.smallSpacing
    transformOrigin: !cascade ? Item.Top : (mirrored ? Item.TopRight : Item.TopLeft)

    delegate: MenuItem { }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            easing.type: Easing.InOutCubic
            duration: 100
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InOutCubic
            duration: 200
        }
    }

    Overlay.modal: Item {
        Rectangle {
            anchors.fill: parent
            color: 'transparent'
        }
    }

    contentItem: ListView {
        implicitHeight: contentHeight

        implicitWidth: {
            var maxWidth = 0;
            for (var i = 0; i < contentItem.children.length; ++i) {
                maxWidth = Math.max(maxWidth, contentItem.children[i].implicitWidth);
            }
            return maxWidth;
        }

        model: control.contentModel
        interactive: Window.window ? contentHeight > Window.window.height : false
        clip: true
        currentIndex: control.currentIndex || 0
        spacing: control.spacing
        keyNavigationEnabled: true
        keyNavigationWraps: true

        ScrollBar.vertical: ScrollBar {}
    }

    background: LingmoUI.RoundedRect {
        radius: LingmoUI.Theme.hugeRadius
        opacity: 1

        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            radius: 32
            samples: 32
            horizontalOffset: 0
            verticalOffset: 0
            color: Qt.rgba(0, 0, 0, 0.11)
        }
    }

    T.Overlay.modal: Rectangle  {
        color: Qt.rgba(control.LingmoUI.Theme.backgroundColor.r,
                       control.LingmoUI.Theme.backgroundColor.g,
                       control.LingmoUI.Theme.backgroundColor.b, 0.4)
        Behavior on opacity {
            NumberAnimation {
                duration: 150
                easing.type: Easing.InOutCubic
            }
        }
    }

    T.Overlay.modeless: Rectangle {
        color: Qt.rgba(control.LingmoUI.Theme.backgroundColor.r,
                       control.LingmoUI.Theme.backgroundColor.g,
                       control.LingmoUI.Theme.backgroundColor.b, 0.4)
        Behavior on opacity {
            NumberAnimation {
                duration: 150
                easing.type: Easing.InOutCubic
            }
        }
    }
}
