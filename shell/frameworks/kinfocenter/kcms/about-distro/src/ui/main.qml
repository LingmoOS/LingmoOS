/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCMUtils

import org.kde.kinfocenter.about_distro.private as Private

KCMUtils.SimpleKCM {
    id: root

    // bugger off with all your buttons, we don't need any!
    KCMUtils.ConfigModule.buttons: KCMUtils.ConfigModule.NoAdditionalButton

    implicitWidth: LingmoUI.Units.gridUnit * 20
    implicitHeight: LingmoUI.Units.gridUnit * 20

    // Scoot away from the edges! Otherwise the KCM looks overly cramped.
    topPadding: LingmoUI.Units.gridUnit
    leftPadding: LingmoUI.Units.gridUnit
    rightPadding: LingmoUI.Units.gridUnit

    Private.ServiceRunner {
        id: kicRunner
        desktopFileName: "systemsettings"
    }

    ColumnLayout {
        spacing: LingmoUI.Units.largeSpacing
        id: layout
        // anchors.fill: parent

        RowLayout {
            id: sysinf
            Layout.alignment: Qt.AlignHCenter
            // spacing: LingmoUI.Units.largeSpacing

            LingmoUI.Icon {
                id: logo
                source: kcm.distroLogo
                implicitWidth: LingmoUI.Units.iconSizes.enormous * 2
                implicitHeight: LingmoUI.Units.iconSizes.enormous * 2
            }
            // Image {
            //     id: logo
            //     anchors {
            //         // top: parent.top
            //         // bottom: parent.bottom
            //         verticalCenter: parent.verticalCenter
            //         right: lbt.left
            //         rightMargin: sysinf.width/30
            //     }
            //     width: 250
            //     sourceSize: Qt.size(width, height)
            //     source: kcm.distroLogo
            // }
            Item {
                width: LingmoUI.Units.largeSpacing * 3
            }

            Rectangle {
                id: lbt
                Layout.alignment: Qt.AlignHCenter
                height: LingmoUI.Units.iconSizes.enormous/3
                width: 2
                color: "#64646E"
                opacity: 0.5
            }

            Item {
                width: LingmoUI.Units.largeSpacing * 3
            }

            ColumnLayout {
                // Item { Layout.fillHeight: true }

                // LingmoUI.Heading {
                //     text: kcm.distroNameVersion
                //     level: 1
                //     type: LingmoUI.Heading.Type.Primary
                // }

                // LingmoUI.Heading {
                //     visible: kcm.distroVariant !== ""
                //     text: kcm.distroVariant
                //     level: 2
                //     type: LingmoUI.Heading.Type.Secondary
                // }

                // QQC2.Label {
                //     visible: kcm.distroUrl !== ""
                //     text: "<a href='%1'>%1</a>".arg(kcm.distroUrl)
                //     textFormat: Text.RichText
                //     onLinkActivated: link => Qt.openUrlExternally(link)

                //     HoverHandler { cursorShape: Qt.PointingHandCursor }
                // }

                Rectangle {
                    id: deviceItem

                    anchors {
                        left: lbt.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: 10
                    }
                    width: LingmoUI.Units.iconSizes.enormous * 2/1.2
                    height: LingmoUI.Units.iconSizes.enormous * 2/2
                    color: "transparent"
                    border.width: 5
                    border.color: LingmoUI.Theme.bgkColor
                    radius: 16

                    Image {
                        id: _image
                        width: deviceItem.width - 8
                        height: deviceItem.height - 8
                        anchors {
                            verticalCenter: parent.verticalCenter
                            horizontalCenter: parent.horizontalCenter
                        }
                        // source: LingmoUI.Theme.darkMode ? "qrc:/images/MundoDark.jpeg" : "qrc:/images/MundoLight.jpeg"
                        source: "file://usr/share/pixmaps/lingmo-next.jpeg"
                        // source: kcm.distroLogo
                        sourceSize: Qt.size(width, height)
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        mipmap: true
                        cache: true
                        smooth: true
                        opacity: 1.0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.InOutCubic
                            }
                        }
                        

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Item {
                                width: _image.width
                                height: _image.height
                                Rectangle {
                                    anchors.fill: parent
                                    radius: 16
                                }
                            }
                        }
                    }
                }

                // Item { Layout.fillHeight: true }
            }
        }

        LingmoUI.RoundedItem {
            LingmoUI.StandardItem {
                key: i18n("Operating System")
                value: kcm.distroNameVersion
            }
            Item {
                id: control
                height: LingmoUI.Units.smallSpacing

                Layout.fillWidth: true

                Rectangle {
                    anchors.centerIn: parent
                    height: 1
                    width: control.width
                    color: LingmoUI.Theme.disabledTextColor
                    opacity: LingmoUI.Theme.darkMode ? 0.7 : 0.5
                }
            }
            LingmoUI.StandardItem {
                key: i18n("Kernel Version")
                value: i18n("Linux ") + kcm.kernelVersion
            }
        }

        Item {
            height: LingmoUI.Units.smallSpacing
        }

        LingmoUI.RoundedItem {
            LingmoUI.StandardItem {
                key: i18n("Desktop Version")
                value: ("%s")
            }
            Item {
                id: control2
                height: LingmoUI.Units.smallSpacing

                Layout.fillWidth: true

                Rectangle {
                    anchors.centerIn: parent
                    height: 1
                    width: control2.width
                    color: LingmoUI.Theme.disabledTextColor
                    opacity: LingmoUI.Theme.darkMode ? 0.7 : 0.5
                }
            }
            LingmoUI.StandardItem {
                key: i18n("UI Version")
                value: i18n("LingmoUI ") + ("VERSION")
            }
        }

        // LingmoUI.FormLayout {
        //     Layout.fillHeight: true

        //     Component {
        //         id: entryComponent
        //         RowLayout {
        //             LingmoUI.FormData.label: entry.localizedLabel()
        //             readonly property bool hidden: entry.isHidden()

        //             // Component {
        //             //     id: unhideDialog
        //             //     // LingmoUI.PromptDialog {
        //             //     //     // NB: should we ever have other entries that need this dialog then this needs refactoring on the Entry side.
        //             //     //     //  Do NOT simply add if else logic here!
        //             //     //     title: i18nc("@title", "Serial Number")
        //             //     //     subtitle: entry.localizedValue()
        //             //     //     flatFooterButtons: true
        //             //     //     standardButtons: LingmoUI.Dialog.NoButton
        //             //     //     customFooterActions: [
        //             //     //         LingmoUI.Action {
        //             //     //             text: i18nc("@action:button", "Copy")
        //             //     //             icon.name: "edit-copy-symbolic"
        //             //     //             onTriggered: source => kcm.storeInClipboard(subtitle)
        //             //     //             shortcut: StandardKey.Copy
        //             //     //         }
        //             //     //     ]
        //             //     //     onClosed: destroy();
        //             //     // }
        //             // }

        //             LingmoUI.SelectableLabel {
        //                 id: valueLabel
        //                 visible: !hidden
        //                 text: entry.localizedValue()
        //                 Keys.onShortcutOverride: event => {
        //                     event.accepted = (valueLabel.activeFocus && valueLabel.selectedText && event.matches(StandardKey.Copy));
        //                 }
        //                 Keys.onPressed: event => {
        //                     if (event.matches(StandardKey.Copy)) {
        //                         valueLabel.copy();
        //                         event.accepted = true;
        //                     }
        //                 }
        //             }

        //             QQC2.Button {
        //                 visible: hidden
        //                 property var dialog: null
        //                 icon.name: "view-visible-symbolic"
        //                 text: i18nc("@action:button show a hidden entry in an overlay", "Show")
        //                 onClicked: {
        //                     if (!dialog) {
        //                         dialog = unhideDialog.createObject(root, {});
        //                     }
        //                     dialog.open();
        //                 }
        //             }
        //         }
        //     }

        //     Item {
        //         LingmoUI.FormData.label: i18nc("@title:group", "Software")
        //         LingmoUI.FormData.isSection: true
        //     }

        //     Repeater {
        //         model: kcm.softwareEntries
        //         delegate: entryComponent
        //     }

        //     Item {
        //         LingmoUI.FormData.label: i18nc("@title:group", "Hardware")
        //         LingmoUI.FormData.isSection: true
        //     }

        //     Repeater {
        //         model: kcm.hardwareEntries
        //         delegate: entryComponent
        //     }
        // }
    }


    actions: [
        LingmoUI.Action {
            visible: !kcm.isThisSystemInfo && kicRunner.canRun

            icon.name: kicRunner.iconName
            text: i18nc("@action:button launches kinfocenter from systemsettings", "More System Information")
            tooltip: i18nc("@info:tooltip", "Launch %1", kicRunner.genericName)
            onTriggered: source => kicRunner.run()
        },

        LingmoUI.Action {
            visible: kcm.isEnglish

            icon.name: "edit-copy-symbolic"
            text: i18nc("@action:button", "Copy Details")
            onTriggered: source => kcm.copyToClipboard()
        },

        LingmoUI.Action {
            visible: !kcm.isEnglish

            icon.name: "edit-copy-symbolic"
            text: i18nc("@action:button", "Copy Details")

            LingmoUI.Action {
                text: i18nc("@action:button Copy Details...", "In current language")
                onTriggered: source => kcm.copyToClipboard()
                shortcut: StandardKey.Copy
            }

            LingmoUI.Action {
                text: i18nc("@action:button Copy Details...", "In English")
                onTriggered: source => kcm.copyToClipboardInEnglish()
            }
        }
    ]
}
