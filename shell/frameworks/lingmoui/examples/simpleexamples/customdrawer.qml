/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

    globalDrawer: LingmoUI.OverlayDrawer {
        contentItem: Rectangle {
            implicitWidth: LingmoUI.Units.gridUnit * 10
            color: "red"
            anchors.fill: parent
        }
    }
    contextDrawer: LingmoUI.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: mainPageComponent

    Component {
        id: mainPageComponent
        LingmoUI.ScrollablePage {
            title: "Hello"
            Rectangle {
                anchors.fill: parent
            }
        }
    }
}
