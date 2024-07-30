/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

LingmoUI.AbstractApplicationWindow {
    id: root

    width: 500
    height: 800
    visible: true

    globalDrawer: LingmoUI.GlobalDrawer {
        title: "Widget gallery"
        titleIcon: "applications-graphics"
        actions: [
            LingmoUI.Action {
                text: "View"
                icon.name: "view-list-icons"
                LingmoUI.Action {
                    text: "action 1"
                }
                LingmoUI.Action {
                    text: "action 2"
                }
                LingmoUI.Action {
                    text: "action 3"
                }
            },
            LingmoUI.Action {
                text: "Sync"
                icon.name: "folder-sync"
                LingmoUI.Action {
                    text: "action 4"
                }
                LingmoUI.Action {
                    text: "action 5"
                }
            },
            LingmoUI.Action {
                text: "Checkable"
                icon.name: "view-list-details"
                checkable: true
                checked: false
                onTriggered: {
                    print("Action checked:" + checked)
                }
            },
            LingmoUI.Action {
                text: "Settings"
                icon.name: "configure"
                checkable: true
                //Need to do this, otherwise it breaks the bindings
                property bool current: pageStack.currentItem?.objectName === "settingsPage" ?? false
                onCurrentChanged: {
                    checked = current;
                }
                onTriggered: {
                    pageStack.push(settingsComponent);
                }
            }
        ]

        QQC2.CheckBox {
            checked: true
            text: "Option 1"
        }
        QQC2.CheckBox {
            text: "Option 2"
        }
        QQC2.CheckBox {
            text: "Option 3"
        }
        QQC2.Slider {
            Layout.fillWidth: true
            value: 0.5
        }
    }
    contextDrawer: LingmoUI.ContextDrawer {
        id: contextDrawer
    }

    pageStack: QQC2.StackView {
        anchors.fill: parent
        property int currentIndex: 0
        focus: true
        onCurrentIndexChanged: {
            if (depth > currentIndex + 1) {
                pop(get(currentIndex));
            }
        }
        onDepthChanged: {
            currentIndex = depth-1;
        }
        initialItem: mainPageComponent

        Keys.onReleased: event => {
            if (event.key === Qt.Key_Back ||
                    (event.key === Qt.Key_Left && (event.modifiers & Qt.AltModifier))) {
                event.accepted = true;
                if (root.contextDrawer && root.contextDrawer.drawerOpen) {
                    root.contextDrawer.close();
                } else if (root.globalDrawer && root.globalDrawer.drawerOpen) {
                    root.globalDrawer.close();
                } else {
                    var backEvent = {accepted: false}
                    if (root.pageStack.currentIndex >= 1) {
                        root.pageStack.currentItem.backRequested(backEvent);
                        if (!backEvent.accepted) {
                            if (root.pageStack.depth > 1) {
                                root.pageStack.currentIndex = Math.max(0, root.pageStack.currentIndex - 1);
                                backEvent.accepted = true;
                            } else {
                                Qt.quit();
                            }
                        }
                    }

                    if (!backEvent.accepted) {
                        Qt.quit();
                    }
                }
            }
        }
    }

    Component {
        id: settingsComponent
        LingmoUI.Page {
            title: "Settings"
            objectName: "settingsPage"
            Rectangle {
                anchors.fill: parent
            }
        }
    }

    //Main app content
    Component {
        id: mainPageComponent
        MultipleColumnsGallery {}
    }
}
