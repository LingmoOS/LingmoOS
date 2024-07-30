/*
 *  SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {

    Component {
        id: delegateComponent
        LingmoUI.Card {
            contentItem: Label { text: ourlist.prefix + index }
        }
    }

    pageStack.initialPage: LingmoUI.Page {
        actions: [
            LingmoUI.Action {
                text: "Switch Icon"
                onTriggered: {
                    if (icon.source === "home") {
                        icon.source = "window-new";
                    } else {
                        icon.source = "home";
                    }
                }
            },
            LingmoUI.Action {
                text: "Enabled"
                checkable: true
                checked: icon.enabled
                onTriggered: icon.enabled = !icon.enabled
            },
            LingmoUI.Action {
                text: "Animated"
                checkable: true
                checked: icon.animated
                onTriggered: icon.animated = !icon.animated
            },
            LingmoUI.Action {
                displayComponent: RowLayout {
                    Label {
                        text: "Size:"
                    }
                    SpinBox {
                        from: 0
                        to: LingmoUI.Units.iconSizes.enormous
                        value: LingmoUI.Units.iconSizes.large
                        onValueModified: {
                            icon.width = value;
                            icon.height = value;
                        }
                    }
                }
            }
        ]

        LingmoUI.Icon {
            id: icon
            width: LingmoUI.Units.iconSizes.Large
            height: width
            source: "home"
        }
    }
}
