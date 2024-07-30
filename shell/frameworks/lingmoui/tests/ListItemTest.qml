/*
 *  SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 *  SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI
import org.kde.lingmoui.delegates as KD

LingmoUI.ApplicationWindow {
    GridLayout {
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.gridUnit

        rows: 3
        rowSpacing: LingmoUI.Units.gridUnit
        columns: 3
        columnSpacing: LingmoUI.Units.gridUnit

        LingmoUI.Theme.inherit: false
        LingmoUI.Theme.colorSet: LingmoUI.Theme.View

        // Icon + Label
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            LingmoUI.Heading {
                text: "Icon + Label"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
            }
            KD.CheckSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
            }
            KD.RadioSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
            }
            KD.SwitchSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
            }
        }

        // Label + space reserved for icon
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            LingmoUI.Heading {
                text: "Icon + Label + space reserved for icon"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                Layout.fillWidth: true
                text: "Boom!"
                icon.width: LingmoUI.Units.iconSizes.smallMedium
            }
            KD.CheckSubtitleDelegate {
                Layout.fillWidth: true
                text: "Boom!"
                icon.width: LingmoUI.Units.iconSizes.smallMedium
            }
            KD.RadioSubtitleDelegate {
                Layout.fillWidth: true
                text: "Boom!"
                icon.width: LingmoUI.Units.iconSizes.smallMedium
            }
            KD.SwitchSubtitleDelegate {
                Layout.fillWidth: true
                text: "Boom!"
                icon.width: LingmoUI.Units.iconSizes.smallMedium
            }
        }

        // Icon + Label + leading and trailing items
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            LingmoUI.Heading {
                text: "Icon + Label + leading and trailing items"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            QQC2.ItemDelegate {
                id: plainDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: LingmoUI.Units.largeSpacing
                        Layout.preferredHeight: LingmoUI.Units.largeSpacing
                        color: LingmoUI.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: plainDelegate.text
                        icon: icon.fromControlsIcon(plainDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            QQC2.CheckDelegate {
                id: checkDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: LingmoUI.Units.largeSpacing
                        Layout.preferredHeight: LingmoUI.Units.largeSpacing
                        color: LingmoUI.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: checkDelegate.text
                        icon: icon.fromControlsIcon(checkDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            QQC2.RadioDelegate {
                id: radioDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: LingmoUI.Units.largeSpacing
                        Layout.preferredHeight: LingmoUI.Units.largeSpacing
                        color: LingmoUI.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: radioDelegate.text
                        icon: icon.fromControlsIcon(radioDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            QQC2.SwitchDelegate {
                id: switchDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: LingmoUI.Units.largeSpacing
                        Layout.preferredHeight: LingmoUI.Units.largeSpacing
                        color: LingmoUI.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: switchDelegate.text
                        icon: icon.fromControlsIcon(switchDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
        }

        // Icon + Label + subtitle
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            LingmoUI.Heading {
                text: "Icon + Label + subtitle"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"
            }
            KD.CheckSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"
            }
            KD.RadioSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"
            }
            KD.SwitchSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"
            }
        }

        // Icon + Label + space reserved for subtitle
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            LingmoUI.Heading {
                text: "Icon + Label + space reserved for subtitle"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: KD.IconTitleSubtitle {
                    title: parent.text
                    icon: icon.fromControlsIcon(parent.icon)
                    reserveSpaceForSubtitle: true
                }
            }
            KD.CheckSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: KD.IconTitleSubtitle {
                    title: parent.text
                    icon: icon.fromControlsIcon(parent.icon)
                    reserveSpaceForSubtitle: true
                }
            }
            KD.RadioSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: KD.IconTitleSubtitle {
                    title: parent.text
                    icon: icon.fromControlsIcon(parent.icon)
                    reserveSpaceForSubtitle: true
                }
            }
            KD.SwitchSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: KD.IconTitleSubtitle {
                    title: parent.text
                    icon: icon.fromControlsIcon(parent.icon)
                    reserveSpaceForSubtitle: true
                }
            }
        }

        // Icon + Label + subtitle + leading and trailing items
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            LingmoUI.Heading {
                text: "Icon + Label + subtitle + leading/trailing"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                id: subtitleDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: LingmoUI.Units.largeSpacing
                        Layout.preferredHeight: LingmoUI.Units.largeSpacing
                        color: LingmoUI.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: subtitleDelegate.text
                        subtitle: subtitleDelegate.subtitle
                        selected: subtitleDelegate.highlighted || subtitleDelegate.down
                        icon: icon.fromControlsIcon(subtitleDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            KD.CheckSubtitleDelegate {
                id: subtitleCheckDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: LingmoUI.Units.largeSpacing
                        Layout.preferredHeight: LingmoUI.Units.largeSpacing
                        color: LingmoUI.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: subtitleCheckDelegate.text
                        subtitle: subtitleCheckDelegate.subtitle
                        selected: subtitleCheckDelegate.highlighted || subtitleCheckDelegate.down
                        icon: icon.fromControlsIcon(subtitleCheckDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            KD.RadioSubtitleDelegate {
                id: subtitleRadioDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: LingmoUI.Units.largeSpacing
                        Layout.preferredHeight: LingmoUI.Units.largeSpacing
                        color: LingmoUI.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: subtitleRadioDelegate.text
                        subtitle: subtitleRadioDelegate.subtitle
                        selected: subtitleRadioDelegate.highlighted || subtitleRadioDelegate.down
                        icon: icon.fromControlsIcon(subtitleRadioDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            KD.SwitchSubtitleDelegate {
                id: subtitleSwitchDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: LingmoUI.Units.largeSpacing
                        Layout.preferredHeight: LingmoUI.Units.largeSpacing
                        color: LingmoUI.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: subtitleSwitchDelegate.text
                        subtitle: subtitleSwitchDelegate.subtitle
                        selected: subtitleSwitchDelegate.highlighted || subtitleSwitchDelegate.down
                        icon: icon.fromControlsIcon(subtitleSwitchDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
        }
    }
}

