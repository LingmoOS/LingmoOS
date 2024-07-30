/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

    LingmoUI.PagePool {
        id: mainPagePool
    }

    globalDrawer: LingmoUI.GlobalDrawer {
    }
    contextDrawer: LingmoUI.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: wideScreen ? [firstPage, mainPagePool.loadPage("SimplePage.qml")] : [firstPage]

    Component {
        id: firstPage
        LingmoUI.ScrollablePage {
            id: root
            title: i18n("Sidebar")
            property list<LingmoUI.PagePoolAction> pageActions: [
                LingmoUI.PagePoolAction {
                    text: i18n("Page1")
                    icon.name: "speedometer"
                    pagePool: mainPagePool
                    basePage: root
                    page: "SimplePage.qml"
                },
                LingmoUI.PagePoolAction {
                    text: i18n("Page2")
                    icon.name: "window-duplicate"
                    pagePool: mainPagePool
                    basePage: root
                    page: "MultipleColumnsGallery.qml"
                }
            ]
            ListView {
                model: pageActions
                keyNavigationEnabled: true
                activeFocusOnTab: true
                reuseItems: true
                delegate: QQC2.ItemDelegate {
                    id: delegate
                    action: modelData
                    width: parent.width
                }
            }
        }
    }
}
