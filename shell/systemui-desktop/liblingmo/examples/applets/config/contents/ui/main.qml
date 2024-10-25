/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.plasmoid
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

PlasmoidItem {
    id: root

    fullRepresentation: ColumnLayout {
        id: column
        spacing: LingmoUI.Units.smallSpacing

        Layout.minimumWidth: LingmoUI.Units.gridUnit * 10
        Layout.minimumHeight: implicitHeight

        Item {
            Layout.fillHeight: true
        }
        LingmoComponents.CheckBox {
            Layout.fillWidth: true
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
            enabled: true
            checked: Plasmoid.configuration.BoolTest
            text: i18n("Bool from config")
            onToggled: {
                Plasmoid.configuration.BoolTest = checked;
            }
        }
        LingmoComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: i18n("String test")
        }
        LingmoComponents.TextField {
            Layout.fillWidth: true
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
            text: Plasmoid.configuration.Test
            onTextEdited: {
                Plasmoid.configuration.Test = text;
            }
        }
        LingmoComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: i18n("String from another group")
        }
        LingmoComponents.TextField {
            Layout.fillWidth: true
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
            text: Plasmoid.configuration.OtherTest
            onTextEdited: {
                Plasmoid.configuration.OtherTest = text;
            }
        }
        LingmoComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: i18n("Enum\ndisplayed as int,\nwritten as string")
        }
        LingmoComponents.TextField {
            Layout.fillWidth: true
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
            text: Plasmoid.configuration.EnumTest
            onTextEdited: {
                Plasmoid.configuration.EnumTest = text;
            }
        }
        LingmoComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: i18n("Integer\nminimum: -1\nmaximum: 100")
        }
        LingmoComponents.SpinBox {
            Layout.fillWidth: true
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
            from: -1
            to: 100
            value: Plasmoid.configuration.IntTest
            onValueModified: {
                Plasmoid.configuration.IntTest = value;
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
