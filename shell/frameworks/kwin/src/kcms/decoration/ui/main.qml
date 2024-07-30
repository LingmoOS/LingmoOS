/*
    SPDX-FileCopyrightText: 2019 Valerio Pilo <vpilo@coldshock.net>
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.newstuff as NewStuff

KCM.AbstractKCM {
    id: root

    title: kcm.name

    framedView: false

    implicitWidth: LingmoUI.Units.gridUnit * 48
    implicitHeight: LingmoUI.Units.gridUnit * 33

    actions: [
        LingmoUI.Action {
            id: borderSizeComboBox
            text: i18nc("Selector label", "Window border size:")

            displayComponent: RowLayout {
                QQC2.ComboBox {
                    id: borderSizeComboBox
                    currentIndex: kcm.borderIndex
                    flat: true
                    model: kcm.borderSizesModel

                    onActivated: kcm.borderIndex = currentIndex

                    KCM.SettingHighlighter {
                        highlight: kcm.borderIndex !== 0
                    }
                }
            }
        },
        LingmoUI.Action {
            icon.name: "configure"
            text: i18nc("button text", "Configure Titlebar Buttons…")

            onTriggered: kcm.push("ConfigureTitlebar.qml")
        },
        NewStuff.Action {
            configFile: "window-decorations.knsrc"
            text: i18nc("@action:button as in, \"Get New Window Decorations\"", "Get New…")

            onEntryEvent: (entry, event) => {
                if (event === NewStuff.Engine.StatusChangedEvent) {
                    kcm.reloadKWinSettings();
                } else if (event === NewStuff.Engine.EntryAdoptedEvent) {
                    kcm.load();
                }
            }
        }
    ]

    Themes {
        id: themes
        anchors.fill: parent

        KCM.SettingStateBinding {
            configObject: kcm.settings
            settingName: "pluginName"
            target: themes
        }
    }
}
