/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019-2022 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    implicitWidth: LingmoUI.Units.gridUnit * 44
    implicitHeight: LingmoUI.Units.gridUnit * 33

    LingmoUI.FormLayout {
        width: parent.width

        anchors {
            top: parent.top
            topMargin: LingmoUI.Units.largeSpacing
        }

        RowLayout {
            LingmoUI.FormData.label: i18n("Theme:")

            Layout.alignment: Qt.AlignCenter
            spacing: LingmoUI.Units.gridUnit * 2

            QQC2.ButtonGroup { id: themeGroup }

            Thumbnail {
                imageSource: kcm.defaultLightLookAndFeel.thumbnail
                text: kcm.defaultLightLookAndFeel.name
                checked: kcm.globalsSettings.lookAndFeelPackage === kcm.defaultLightLookAndFeel.id
                QQC2.ButtonGroup.group: themeGroup

                onToggled: kcm.globalsSettings.lookAndFeelPackage = kcm.defaultLightLookAndFeel.id

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "lookAndFeelPackage"
                }
            }
            Thumbnail {
                imageSource: kcm.defaultDarkLookAndFeel.thumbnail
                text: kcm.defaultDarkLookAndFeel.name
                checked: kcm.globalsSettings.lookAndFeelPackage === kcm.defaultDarkLookAndFeel.id
                QQC2.ButtonGroup.group: themeGroup

                onToggled: kcm.globalsSettings.lookAndFeelPackage = kcm.defaultDarkLookAndFeel.id

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "lookAndFeelPackage"
                }
            }
        }

        Item {
            LingmoUI.FormData.isSection: false
        }

        RowLayout {
            id: appearanceButtonsRow
            Layout.fillWidth: true
            spacing: LingmoUI.Units.smallSpacing
            LingmoUI.FormData.label: i18nc("@title:group translate as short as possible", "More appearance settings:")

            MostUsedIcon {
                id: wallpaperKCMButton
                Layout.fillWidth: true
                Layout.preferredWidth: 50 // 50% of the available width
                kcmIcon: "preferences-desktop-wallpaper"
                kcmName: i18nc("@action:button as in, 'this will take you to the Wallpaper KCM'", "Wallpaper")
                onClicked: kcm.openKCM("kcm_wallpaper")
            }

            MostUsedIcon {
                Layout.fillWidth: true
                Layout.preferredWidth: 50 // 50% of the available width
                kcmIcon: "preferences-desktop-theme-global"
                kcmName: i18nc("@action:button as in, 'this will take you to the first KCM in the Colors & Themes group'", "Colors & Themes")
                onClicked: kcm.openKCM("kcm_lookandfeel")
            }
        }

        LingmoUI.Separator {
            LingmoUI.FormData.isSection: true
        }

        // We want to show the slider in a logarithmic way. ie
        // move from 4x, 3x, 2x, 1x, 0.5x, 0.25x, 0.125x
        // 0 is a special case, which means "instant speed / no animations"
        ColumnLayout {
            Layout.fillWidth: true

            LingmoUI.FormData.label: slider.Accessible.name
            LingmoUI.FormData.buddyFor: slider

            QQC2.Slider {
                id: slider
                Layout.fillWidth: true
                from: -4
                to: 4
                stepSize: 0.5
                snapMode: QQC2.Slider.SnapAlways
                onMoved: kcm.globalsSettings.animationDurationFactor =
                    (value === to) ? 0 : (1.0 / Math.pow(2, value))
                value: (kcm.globalsSettings.animationDurationFactor === 0)
                    ? slider.to
                    : -(Math.log(kcm.globalsSettings.animationDurationFactor) / Math.log(2))

                Accessible.name: i18n("Animation speed:")

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "animationDurationFactor"
                }
            }
            RowLayout {
                QQC2.Label {
                    text: i18nc("Animation speed", "Slow")
                    textFormat: Text.PlainText
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18nc("Animation speed", "Instant")
                    textFormat: Text.PlainText
                }
            }
        }

        Item {
            LingmoUI.FormData.isSection: false
        }

        // Click behavior settings
        QQC2.ButtonGroup { id: singleClickGroup }

        ColumnLayout {
            LingmoUI.FormData.label: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
            LingmoUI.FormData.buddyFor: doubleClick

            Layout.fillWidth: true
            spacing: 0

            QQC2.RadioButton {
                id: doubleClick
                text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
                checked: !kcm.globalsSettings.singleClick
                onToggled: kcm.globalsSettings.singleClick = false
                QQC2.ButtonGroup.group: singleClickGroup

                Accessible.description: i18n("Open by double-clicking instead")

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "singleClick"
                    extraEnabledConditions: singleClick.enabled
                }
            }

            QQC2.Label {
                Layout.fillWidth: true
                leftPadding: doubleClick.indicator.width
                text: doubleClick.Accessible.description
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: LingmoUI.Theme.smallFont
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.RadioButton {
                id: singleClick
                text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
                checked: kcm.globalsSettings.singleClick
                onToggled: kcm.globalsSettings.singleClick = true
                QQC2.ButtonGroup.group: singleClickGroup

                Accessible.description: i18n("Select by clicking on item's selection marker")

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "singleClick"
                }
            }

            QQC2.Label {
                Layout.fillWidth: true
                leftPadding: singleClick.indicator.width
                text: singleClick.Accessible.description
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: LingmoUI.Theme.smallFont
            }
        }

        Item {
            LingmoUI.FormData.isSection: false
        }


        MostUsedIcon {
            LingmoUI.FormData.label: i18nc("@title:group translate as short as possible", "More behavior settings:")
            Layout.preferredWidth: wallpaperKCMButton.width
            kcmName: i18nc("@action:button as in, 'this will take you to the General Behavior KCM'", "General Behavior")
            kcmIcon: "preferences-desktop"
            onClicked: kcm.openKCM("kcm_workspace")
        }

        LingmoUI.Separator {
            LingmoUI.FormData.isSection: true
            visible: mostUsedGrid.visible
        }

        GridLayout {
            id: mostUsedGrid
            LingmoUI.FormData.label: i18n("Most Used Pages:")

            visible: recentlyUsedRepeater.count > 0
            Layout.fillWidth: true
            rows: 3
            columns: 2
            rowSpacing: LingmoUI.Units.smallSpacing
            columnSpacing: LingmoUI.Units.smallSpacing

            LingmoUI.SizeGroup {
                id: mostUsedPagesSizeGroup
                mode: LingmoUI.SizeGroup.Width
            }

            Repeater {
                id: recentlyUsedRepeater

                model: kcm.mostUsedModel
                delegate: MostUsedIcon {
                    required property var model

                    kcmIcon: model.decoration
                    kcmName: model.display
                    onClicked: kcm.openKCM(model.kcmPlugin)
                }

                onItemAdded: (index, item) => {
                    if (index === 0) {
                        // Set buddy once, for an item in the first row.
                        // No, it doesn't work as a binding on children list.
                        mostUsedGrid.LingmoUI.FormData.buddyFor = item;
                    }
                    mostUsedPagesSizeGroup.items.push(item);
                }
            }
        }
    }
}
