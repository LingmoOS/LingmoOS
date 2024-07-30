/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI
import "private" as P
import "templates" as T


//TODO KF6: remove
/**
 * @brief An item that can be used as a title for the application.
 *
 * Scrolling the main page will make it taller or shorter (through the point of going away)
 * It's a behavior similar to the typical mobile web browser addressbar
 * the minimum, preferred and maximum heights of the item can be controlled with
 * * ``minimumHeight``: default is 0, i.e. hidden
 * * ``preferredHeight``: default is LingmoUI.Units.gridUnit * 1.6
 * * ``maximumHeight``: default is LingmoUI.Units.gridUnit * 3
 *
 * To achieve a titlebar that stays completely fixed just set the 3 sizes as the same
 *
 * @inherit org::kde::lingmoui::templates::AbstractApplicationHeader
 */
T.AbstractApplicationHeader {
    id: root

    LingmoUI.Theme.inherit: false
    LingmoUI.Theme.colorSet: LingmoUI.Theme.Header

    background: Rectangle {
        color: LingmoUI.Theme.backgroundColor
        P.EdgeShadow {
            id: shadow
            visible: root.separatorVisible
            anchors {
                right: parent.right
                left: parent.left
                top: parent.bottom
            }
            edge: Qt.TopEdge
            opacity: (!root.page || !root.page.header || root.page.header.toString().indexOf("ToolBar") === -1)
            Behavior on opacity {
                OpacityAnimator {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        Behavior on opacity {
            OpacityAnimator {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
}
