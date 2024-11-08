/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Components 3 RoundButton"
    contentItem: ColumnLayout {
        GridLayout {
            rowSpacing: LingmoUI.Units.smallSpacing
            columnSpacing: LingmoUI.Units.gridUnit
            columns: 2

            LingmoComponents.Label {
                text: "icon + text"
            }

            LingmoComponents.RoundButton {
                icon.name: "start-here-kde-lingmo"
                text: "test"
            }

            LingmoComponents.Label {
                text: "icon alone, should look small and square"
            }

            LingmoComponents.RoundButton {
                icon.name: "start-here-kde-lingmo"
            }

            LingmoComponents.Label {
                text: "text alone, should be about as wide as the text itself"
            }

            LingmoComponents.RoundButton {
                text: "test"
            }

            LingmoComponents.Label {
                text: "This should look highlighted on load"
            }

            LingmoComponents.RoundButton {
                text: "test"
                focus: true
            }

            LingmoComponents.Label {
                text: "long text, should expand to fit"
            }

            LingmoComponents.RoundButton {
                icon.name: "start-here-kde-lingmo"
                text: "This is a really really really really long button"
            }

            LingmoComponents.Label {
                text: "long text but constrained, should be 150px and elided"
            }

            LingmoComponents.RoundButton {
                icon.name: "start-here-kde-lingmo"
                text: "This is a really really really really long button"
                Layout.maximumWidth: 150
            }


            LingmoComponents.Label {
                text: "button (with or without icon) and textfield should have the same height"
            }

            RowLayout {
                LingmoComponents.RoundButton {
                    text: "test"
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "test"
                }
                LingmoComponents.TextField {
                }
            }

            LingmoComponents.Label {
                text: "minimum width property. Should be two letters wide"
            }

            RowLayout {
                LingmoComponents.RoundButton {
                    text: "AA"
                    implicitWidth: Layout.minimumWidth
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "AA"
                    implicitWidth: Layout.minimumWidth
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    implicitWidth: Layout.minimumWidth
                }
            }

            LingmoComponents.Label {
                text: "Display property"
            }
            RowLayout {
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Icon Only"
                    display: LingmoComponents.RoundButton.IconOnly
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Beside Icon"
                    display: LingmoComponents.RoundButton.TextBesideIcon
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Under Icon"
                    display: LingmoComponents.RoundButton.TextUnderIcon
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Only"
                    display: LingmoComponents.RoundButton.TextOnly
                }
            }

        }
        LingmoUI.Heading {
            text: "Flat Buttons"
        }
        GridLayout {
            rowSpacing: LingmoUI.Units.smallSpacing
            columnSpacing: LingmoUI.Units.gridUnit
            columns: 2

            LingmoComponents.Label {
                text: "icon + text"
            }

            LingmoComponents.RoundButton {
                icon.name: "start-here-kde-lingmo"
                text: "test"
                flat: true
            }

            LingmoComponents.Label {
                text: "icon alone, should look small and square"
            }

            LingmoComponents.RoundButton {
                icon.name: "start-here-kde-lingmo"
                flat: true
            }

            LingmoComponents.Label {
                text: "text alone, should be about as wide as the text itself"
            }

            LingmoComponents.RoundButton {
                text: "test"
                flat: true
            }

            LingmoComponents.Label {
                text: "long text, should expand to fit"
            }

            LingmoComponents.RoundButton {
                icon.name: "start-here-kde-lingmo"
                text: "This is a really really really really long button"
                flat: true
            }

            LingmoComponents.Label {
                text: "long text but constrained, should be 150px and elided"
            }

            LingmoComponents.RoundButton {
                icon.name: "start-here-kde-lingmo"
                text: "This is a really really really really long button"
                Layout.maximumWidth: 150
                flat: true
            }


            LingmoComponents.Label {
                text: "button (with or without icon) and textfield should have the same height"
            }

            RowLayout {
                LingmoComponents.RoundButton {
                    text: "test"
                    flat: true
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "test"
                    flat: true
                }
                LingmoComponents.TextField {
                }
            }

            LingmoComponents.Label {
                text: "minimum width property. Should be two letters wide"
            }

            RowLayout {
                LingmoComponents.RoundButton {
                    text: "AA"
                    implicitWidth: Layout.minimumWidth
                    flat: true
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "AA"
                    implicitWidth: Layout.minimumWidth
                    flat: true
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    implicitWidth: Layout.minimumWidth
                    flat: true
                }
            }

            LingmoComponents.Label {
                text: "Display property"
            }
            RowLayout {
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Icon Only"
                    display: LingmoComponents.RoundButton.IconOnly
                    flat: true
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Beside Icon"
                    display: LingmoComponents.RoundButton.TextBesideIcon
                    flat: true
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Under Icon"
                    display: LingmoComponents.RoundButton.TextUnderIcon
                    flat: true
                }
                LingmoComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Only"
                    display: LingmoComponents.RoundButton.TextOnly
                    flat: true
                }
            }
        }
    }
}
