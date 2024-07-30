/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI
import QtQuick.Controls as QQC2

LingmoUI.ApplicationWindow {
    id: root

    width: LingmoUI.Units.gridUnit * 60
    height: LingmoUI.Units.gridUnit * 40

    pageStack.initialPage: mainPageComponent
    globalDrawer: LingmoUI.OverlayDrawer {
        drawerOpen: true
        modal: false
        contentItem: Item {
            implicitWidth: LingmoUI.Units.gridUnit * 10

            QQC2.Label {
                text: "This is a sidebar"
                width: parent.width - LingmoUI.Units.smallSpacing * 2
                wrapMode: Text.WordWrap
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    //Main app content
    Component {
        id: mainPageComponent
        MultipleColumnsGallery {}
    }
}
