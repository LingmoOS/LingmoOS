/*
 *  SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

    //FIXME: perhaps the default logic for going widescreen should be refined upstream
    wideScreen: width > columnWidth * 3
    property int columnWidth: LingmoUI.Units.gridUnit * 13
    property int footerHeight: Math.round(LingmoUI.Units.gridUnit * 2.5)

    globalDrawer: LingmoUI.GlobalDrawer {
        contentItem.implicitWidth: columnWidth
        modal: true
        drawerOpen: false
        isMenu: true

        actions: [
            LingmoUI.Action {
                text: "Rooms"
                icon.name: "view-list-icons"
            },
            LingmoUI.Action {
                text: "Contacts"
                icon.name: "tag-people"
            },
            LingmoUI.Action {
                text: "Search"
                icon.name: "search"
            }
        ]
    }
    contextDrawer: LingmoUI.OverlayDrawer {
        id: contextDrawer
        //they can depend on the page like that or be defined directly here
        edge: Qt.application.layoutDirection === Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge
        modal: !root.wideScreen
        onModalChanged: drawerOpen = !modal
        handleVisible: root.controlsVisible

        //here padding 0 as listitems look better without as opposed to any other control
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding: 0

        contentItem: ColumnLayout {
            readonly property int implicitWidth: root.columnWidth
            spacing: 0
            QQC2.Control {
                Layout.fillWidth: true
                background: Rectangle {
                    anchors.fill: parent
                    color: LingmoUI.Theme.highlightColor
                    opacity: 0.8
                }

                padding: LingmoUI.Units.gridUnit

                contentItem: ColumnLayout {
                    id: titleLayout
                    spacing: LingmoUI.Units.gridUnit

                    RowLayout {
                        spacing: LingmoUI.Units.gridUnit
                        Rectangle {
                            color: LingmoUI.Theme.highlightedTextColor
                            radius: width
                            implicitWidth: LingmoUI.Units.iconSizes.medium
                            implicitHeight: implicitWidth
                        }
                        ColumnLayout {
                            QQC2.Label {
                                Layout.fillWidth: true
                                color: LingmoUI.Theme.highlightedTextColor
                                text: "KDE"
                            }
                            QQC2.Label {
                                Layout.fillWidth: true
                                color: LingmoUI.Theme.highlightedTextColor
                                font: LingmoUI.Theme.smallFont
                                text: "#kde: kde.org"
                            }
                        }
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        color: LingmoUI.Theme.highlightedTextColor
                        text: "Main room for KDE community, other rooms are listed at kde.org/rooms"
                        wrapMode: Text.WordWrap
                    }
                }
            }

            LingmoUI.Separator {
                Layout.fillWidth: true
            }

            QQC2.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                ListView {
                    reuseItems: true
                    model: 50
                    delegate: QQC2.ItemDelegate {
                        text: "Person " + modelData
                        width: parent.width
                    }
                }
            }

            LingmoUI.Separator {
                Layout.fillWidth: true
                Layout.maximumHeight: 1//implicitHeight
            }
            QQC2.ItemDelegate {
                text: "Group call"
                icon.name: "call-start"
                width: parent.width
            }
            QQC2.ItemDelegate {
                text: "Send Attachment"
                icon.name: "mail-attachment"
                width: parent.width
            }
        }
    }

    pageStack.defaultColumnWidth: columnWidth
    pageStack.initialPage: [channelsComponent, chatComponent]

    Component {
        id: channelsComponent
        LingmoUI.ScrollablePage {
            title: "Channels"
            actions: LingmoUI.Action {
                icon.name: "search"
                text: "Search"
            }
            background: Rectangle {
                anchors.fill: parent
                color: LingmoUI.Theme.backgroundColor
            }
            footer: QQC2.ToolBar {
                height: root.footerHeight
                padding: LingmoUI.Units.smallSpacing
                RowLayout {
                    anchors.fill: parent
                    spacing: LingmoUI.Units.smallSpacing
                    QQC2.ToolButton {
                        Layout.fillHeight: true
                        //make it square
                        implicitWidth: height
                        icon.name: "configure"
                        onClicked: root.pageStack.layers.push(secondLayerComponent);
                    }
                    QQC2.ComboBox {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: ["First", "Second", "Third"]
                    }
                }
            }
            ListView {
                id: channelsList
                currentIndex: 2
                model: 30
                reuseItems: true
                delegate: QQC2.ItemDelegate {
                    text: "#Channel " + modelData
                    checkable: true
                    checked: channelsList.currentIndex === index
                    width: parent.width
                }
            }
        }
    }

    Component {
        id: chatComponent
        LingmoUI.ScrollablePage {
            title: "#KDE"
            actions: [
                LingmoUI.Action {
                    icon.name: "documentinfo"
                    text: "Channel info"
                },
                LingmoUI.Action {
                    icon.name: "search"
                    text: "Search"
                },
                LingmoUI.Action {
                    text: "Room Settings"
                    icon.name: "configure"
                    LingmoUI.Action {
                        text: "Setting 1"
                    }
                    LingmoUI.Action {
                        text: "Setting 2"
                    }
                },
                LingmoUI.Action {
                    text: "Shared Media"
                    icon.name: "document-share"
                    LingmoUI.Action {
                        text: "Media 1"
                    }
                    LingmoUI.Action {
                        text: "Media 2"
                    }
                    LingmoUI.Action {
                        text: "Media 3"
                    }
                }
            ]
            background: Rectangle {
                anchors.fill: parent
                color: LingmoUI.Theme.backgroundColor
            }
            footer: QQC2.Control {
                height: footerHeight
                padding: LingmoUI.Units.smallSpacing
                background: Rectangle {
                    color: LingmoUI.Theme.backgroundColor
                    LingmoUI.Separator {
                        Rectangle {
                            anchors.fill: parent
                            color: LingmoUI.Theme.focusColor
                            visible: chatTextInput.activeFocus
                        }
                        anchors {
                            left: parent.left
                            right: parent.right
                            top: parent.top
                        }
                    }
                }
                contentItem: RowLayout {
                    QQC2.TextField {
                        Layout.fillWidth: true
                        id: chatTextInput
                        background: Item {}
                    }
                    QQC2.ToolButton {
                        Layout.fillHeight: true
                        //make it square
                        implicitWidth: height
                        icon.name: "go-next"
                    }
                }
            }

            ListView {
                id: channelsList
                verticalLayoutDirection: ListView.BottomToTop
                currentIndex: 2
                model: 30
                reuseItems: true
                delegate: Item {
                    height: LingmoUI.Units.gridUnit * 4
                    ColumnLayout {
                        x: LingmoUI.Units.gridUnit
                        anchors.verticalCenter: parent.verticalCenter
                        QQC2.Label {
                            text: modelData % 2 ? "John Doe" : "John Applebaum"
                        }
                        QQC2.Label {
                            text: "Message " + modelData
                        }
                    }
                }
            }
        }
    }

    Component {
        id: secondLayerComponent
        LingmoUI.Page {
            title: "Settings"
            background: Rectangle {
                color: LingmoUI.Theme.backgroundColor
            }
            footer: QQC2.ToolBar {
                height: root.footerHeight
                QQC2.ToolButton {
                    Layout.fillHeight: true
                    //make it square
                    implicitWidth: height
                    icon.name: "configure"
                    onClicked: root.pageStack.layers.pop();
                }
            }
        }
    }
}
