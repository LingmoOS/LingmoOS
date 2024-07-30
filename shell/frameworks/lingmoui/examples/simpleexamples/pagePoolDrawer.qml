/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

    LingmoUI.PagePool {
        id: mainPagePool
    }

    globalDrawer: LingmoUI.GlobalDrawer {
        modal: !root.wideScreen
        width: LingmoUI.Units.gridUnit * 10

        actions: [
            LingmoUI.PagePoolAction {
                text: i18n("Page1")
                icon.name: "speedometer"
                pagePool: mainPagePool
                page: "SimplePage.qml"
            },
            LingmoUI.PagePoolAction {
                text: i18n("Page2")
                icon.name: "window-duplicate"
                pagePool: mainPagePool
                page: "MultipleColumnsGallery.qml"
            }
        ]
    }
    contextDrawer: LingmoUI.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: mainPagePool.loadPage("SimplePage.qml")

}
