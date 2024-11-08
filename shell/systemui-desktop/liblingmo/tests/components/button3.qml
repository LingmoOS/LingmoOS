/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Marco Martin <notmart@gmail.com>
 * SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "LingmoComponents 3 Button"
    contentItem: ColumnLayout {
        GridLayout {
            rowSpacing: LingmoUI.Units.smallSpacing
            columnSpacing: LingmoUI.Units.gridUnit
            columns: 2

            LingmoComponents.Label {
                text: "icon + text"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                text: "test"
            }

            LingmoComponents.Label {
                text: "icon alone, should look small and square"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
            }

            LingmoComponents.Label {
                text: "text alone, should be about as wide as the text itself"
            }

            LingmoComponents.Button {
                text: "test"
            }

            LingmoComponents.Label {
                text: "This should look highlighted on load"
            }

            LingmoComponents.Button {
                text: "test"
                focus: true
            }

            LingmoComponents.Label {
                text: "long text, should expand to fit"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                text: "This is a really really really really long button"
            }

            LingmoComponents.Label {
                text: "long text but constrained, should be 150px and elided"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                text: "This is a really really really really long button"
                Layout.maximumWidth: 150
            }

            LingmoComponents.Label {
                text: "disabled icon + text"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                text: "test"
                enabled: false
            }

            LingmoComponents.Label {
                text: "button (with or without icon) and textfield should have the same height"
            }

            RowLayout {
                LingmoComponents.Button {
                    text: "test"
                }
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "test"
                }
                LingmoComponents.TextField {
                }
            }

            LingmoComponents.Label {
                text: "Display property"
            }
            RowLayout {
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "Icon Only"
                    display: LingmoComponents.Button.IconOnly
                }
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "Text Beside Icon"
                    display: LingmoComponents.Button.TextBesideIcon
                }
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "Text Under Icon"
                    display: LingmoComponents.Button.TextUnderIcon
                }
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "Text Only"
                    display: LingmoComponents.Button.TextOnly
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

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                text: "test"
                flat: true
            }

            LingmoComponents.Label {
                text: "icon alone, should look small and square"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                flat: true
            }

            LingmoComponents.Label {
                text: "text alone, should be about as wide as the text itself"
            }

            LingmoComponents.Button {
                text: "test"
                flat: true
            }

            LingmoComponents.Label {
                text: "long text, should expand to fit"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                text: "This is a really really really really long button"
                flat: true
            }

            LingmoComponents.Label {
                text: "long text but constrained, should be 150px and elided"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                text: "This is a really really really really long button"
                Layout.maximumWidth: 150
                flat: true
            }

            LingmoComponents.Label {
                text: "disabled icon + text"
            }

            LingmoComponents.Button {
                icon.name: "start-here-kde-lingmo"
                text: "test"
                flat: true
                enabled: false
            }

            LingmoComponents.Label {
                text: "button (with or without icon) and textfield should have the same height"
            }

            RowLayout {
                LingmoComponents.Button {
                    text: "test"
                    flat: true
                }
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "test"
                    flat: true
                }
                LingmoComponents.TextField {
                }
            }

            LingmoComponents.Label {
                text: "Display property"
            }
            RowLayout {
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "Icon Only"
                    display: LingmoComponents.Button.IconOnly
                    flat: true
                }
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "Text Beside Icon"
                    display: LingmoComponents.Button.TextBesideIcon
                    flat: true
                }
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "Text Under Icon"
                    display: LingmoComponents.Button.TextUnderIcon
                    flat: true
                }
                LingmoComponents.Button {
                    icon.name: "application-menu"
                    text: "Text Only"
                    display: LingmoComponents.Button.TextOnly
                    flat: true
                }
            }
        }
    }
}

