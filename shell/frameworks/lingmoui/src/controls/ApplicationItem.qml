/*
 *  SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

/**
 * @brief An item that provides the features of ApplicationWindow without the window itself.
 *
 * This allows embedding into a larger application.
 * It's based around the PageRow component that allows adding/removing of pages.
 *
 * Example usage:
 * @code
 * import org.kde.lingmoui as LingmoUI
 *
 * LingmoUI.ApplicationItem {
 *     globalDrawer: LingmoUI.GlobalDrawer {
 *         actions: [
 *            LingmoUI.Action {
 *                text: "View"
 *                icon.name: "view-list-icons"
 *                LingmoUI.Action {
 *                        text: "action 1"
 *                }
 *                LingmoUI.Action {
 *                        text: "action 2"
 *                }
 *                LingmoUI.Action {
 *                        text: "action 3"
 *                }
 *            },
 *            LingmoUI.Action {
 *                text: "Sync"
 *                icon.name: "folder-sync"
 *            }
 *         ]
 *     }
 *
 *     contextDrawer: LingmoUI.ContextDrawer {
 *         id: contextDrawer
 *     }
 *
 *     pageStack.initialPage: LingmoUI.Page {
 *         mainAction: LingmoUI.Action {
 *             icon.name: "edit"
 *             onTriggered: {
 *                 // do stuff
 *             }
 *         }
 *         contextualActions: [
 *             LingmoUI.Action {
 *                 icon.name: "edit"
 *                 text: "Action text"
 *                 onTriggered: {
 *                     // do stuff
 *                 }
 *             },
 *             LingmoUI.Action {
 *                 icon.name: "edit"
 *                 text: "Action text"
 *                 onTriggered: {
 *                     // do stuff
 *                 }
 *             }
 *         ]
 *         // ...
 *     }
 * }
 * @endcode
*/
LingmoUI.AbstractApplicationItem {
    id: root

    /**
     * @brief This property holds the PageRow used to allocate the pages and
     * manage the transitions between them.
     *
     * It's using a PageRow, while having the same API as PageStack,
     * it positions the pages as adjacent columns, with as many columns
     * as can fit in the screen. An handheld device would usually have a single
     * fullscreen column, a tablet device would have many tiled columns.
     *
     * @property org::kde::lingmoui::PageRow pageStack
     */
    readonly property alias pageStack: __pageStack

    // Redefines here as here we can know a pointer to PageRow
    wideScreen: width >= applicationWindow().pageStack.defaultColumnWidth * 2

    Component.onCompleted: {
        pageStack.currentItem?.forceActiveFocus();
    }

    LingmoUI.PageRow {
        id: __pageStack
        anchors {
            fill: parent
        }

        function goBack() {
            // NOTE: drawers are handling the back button by themselves
            const backEvent = {accepted: false}
            if (root.pageStack.currentIndex >= 1) {
                root.pageStack.currentItem.backRequested(backEvent);
                if (!backEvent.accepted) {
                    root.pageStack.flickBack();
                    backEvent.accepted = true;
                }
            }

            if (LingmoUI.Settings.isMobile && !backEvent.accepted && Qt.platform.os !== "ios") {
                Qt.quit();
            }
        }
        function goForward() {
            root.pageStack.currentIndex = Math.min(root.pageStack.depth - 1, root.pageStack.currentIndex + 1);
        }
        Keys.onBackPressed: event => {
            goBack();
            event.accepted = true;
        }
        Shortcut {
            sequences: [StandardKey.Forward]
            onActivated: __pageStack.goForward();
        }
        Shortcut {
            sequences: [StandardKey.Back]
            onActivated: __pageStack.goBack();
        }

        background: Rectangle {
            color: root.color
        }

        focus: true
    }
}
