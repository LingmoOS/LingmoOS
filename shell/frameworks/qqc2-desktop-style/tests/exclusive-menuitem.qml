/*
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

QQC2.ApplicationWindow {
    visible: true
    title: "Menu test"
    width: 500
    height: 500

    QQC2.ActionGroup {
        id: actionGroupExclusive
        exclusive: true
    }

    QQC2.ActionGroup {
        id: actionGroupNonExclusive
        exclusive: false
    }

    QQC2.ButtonGroup {
        id: buttonGroupExclusive
        exclusive: true
    }

    QQC2.ButtonGroup {
        id: buttonGroupNonExclusive
        exclusive: false
    }

    QQC2.Menu {
        id: menu

        title: "Top level menu"

        QQC2.Menu {
            title: "Action menu exclusive"
            QQC2.MenuItem {
                action: QQC2.Action {
                    text: "Action one"
                    checkable: true
                    QQC2.ActionGroup.group: actionGroupExclusive
                }
            }
            QQC2.MenuItem {
                action: QQC2.Action {
                    text: "Action two"
                    checkable: true
                    QQC2.ActionGroup.group: actionGroupExclusive
                }
            }
            QQC2.MenuItem {
                action: QQC2.Action {
                    text: "Action three"
                    checkable: true
                    QQC2.ActionGroup.group: actionGroupExclusive
                }
            }
        }

        QQC2.Menu {
            title: "Action menu non-exclusive"
            QQC2.MenuItem {
                action: QQC2.Action {
                    text: "Action one"
                    checkable: true
                    QQC2.ActionGroup.group: actionGroupNonExclusive
                }
            }
            QQC2.MenuItem {
                action: QQC2.Action {
                    text: "Action two"
                    checkable: true
                    QQC2.ActionGroup.group: actionGroupNonExclusive
                }
            }
            QQC2.MenuItem {
                action: QQC2.Action {
                    text: "Action three"
                    checkable: true
                    QQC2.ActionGroup.group: actionGroupNonExclusive
                }
            }
        }

        QQC2.Menu {
            title: "Button menu exclusive"
            QQC2.MenuItem {
                text: "Button one"
                checkable: true
                QQC2.ButtonGroup.group: buttonGroupExclusive
            }
            QQC2.MenuItem {
                text: "Button two"
                checkable: true
                QQC2.ButtonGroup.group: buttonGroupExclusive
            }
            QQC2.MenuItem {
                text: "Button three"
                checkable: true
                QQC2.ButtonGroup.group: buttonGroupExclusive
            }
        }

        QQC2.Menu {
            title: "Button menu non-exclusive"
            QQC2.MenuItem {
                text: "Button one"
                checkable: true
                QQC2.ButtonGroup.group: buttonGroupNonExclusive
            }
            QQC2.MenuItem {
                text: "Button two"
                checkable: true
                QQC2.ButtonGroup.group: buttonGroupNonExclusive
            }
            QQC2.MenuItem {
                text: "Button three"
                checkable: true
                QQC2.ButtonGroup.group: buttonGroupNonExclusive
            }
        }
    }

    QQC2.Button {
        anchors.centerIn: parent
        text: "Open menu"
        onClicked: menu.popup(this, 0, height)
    }
}
