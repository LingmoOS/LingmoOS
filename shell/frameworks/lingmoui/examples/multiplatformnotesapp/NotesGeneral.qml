/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import org.kde.lingmoui.delegates as KD

LingmoUI.ApplicationWindow {
    id: root

    property string currentFile

    pageStack.initialPage: iconView

    LingmoUI.ScrollablePage {
        id: iconView
        title: "Notes"
        actions: LingmoUI.Action {
            id: sortAction
            icon.name: "view-sort-ascending-symbolic"
            tooltip: "Sort Ascending"
        }

        background: Rectangle {
            color: LingmoUI.Theme.backgroundColor
        }

        GridView {
            id: view
            model: 100
            cellWidth: LingmoUI.Units.gridUnit * 9
            cellHeight: cellWidth
            currentIndex: -1
            highlightMoveDuration: 0
            highlight: Rectangle {
                color: LingmoUI.Theme.highlightColor
            }
            delegate: MouseArea {
                id: delegate

                required property int index
                required property int modelData

                width: view.cellWidth
                height: view.cellHeight
                LingmoUI.Icon {
                    source: "text-plain"
                    anchors {
                        fill: parent
                        margins: LingmoUI.Units.gridUnit
                    }
                    QQC2.Label {
                        anchors {
                            top: parent.bottom
                            horizontalCenter: parent.horizontalCenter
                        }
                        text: "File " + delegate.modelData
                    }
                }
                onClicked: {
                    view.currentIndex = index;
                    root.currentFile = "File " + delegate.modelData;
                    if (root.pageStack.depth < 2) {
                        root.pageStack.push(editorComponent);
                    }
                    root.pageStack.currentIndex = 1
                }
            }
        }
    }

    Component {
        id: editorComponent
        LingmoUI.ScrollablePage {
            id: editor
            title: root.currentFile
            actions: [
                LingmoUI.Action {
                    id: shareAction
                    icon.name: "document-share"
                    text: "Share…"
                    tooltip: "Share this document with your device"
                    checked: sheet.visible
                    checkable: true
                    onCheckedChanged: checked => {
                        sheet.visible = checked;
                    }
                },
                LingmoUI.Action {
                    icon.name: "format-text-bold-symbolic"
                    tooltip: "Bold"
                },
                LingmoUI.Action {
                    icon.name: "format-text-underline-symbolic"
                    tooltip: "Underline"
                },
                LingmoUI.Action {
                    icon.name: "format-text-italic-symbolic"
                    tooltip: "Italic"
                }
            ]
            background: Rectangle {
                color: LingmoUI.ColorUtils.brightnessForColor(LingmoUI.Theme.backgroundColor) === LingmoUI.ColorUtils.Light
                    ? "#F8EBC3" : "#1F2226"
            }

            LingmoUI.OverlaySheet {
                id: sheet

                parent: root.QQC2.Overlay.overlay

                onOpened: forceActiveFocus(Qt.PopupFocusReason)

                ListView {
                    implicitWidth: LingmoUI.Units.gridUnit * 30
                    reuseItems: true
                    model: ListModel {
                        ListElement {
                            title: "Share with phone \"Nokia 3310\""
                            description: "You selected this phone 12 times before. It's currently connected via bluetooth"
                            buttonText: "Push Sync"
                        }
                        ListElement {
                            title: "Share with phone \"My other Nexus5\""
                            description: "You selected this phone 0 times before. It's currently connected to your laptop via Wifi"
                            buttonText: "Push Sync"
                        }
                        ListElement {
                            title: "Share with NextCloud"
                            description: "You currently do not have a server set up for sharing and storing notes from Katie. If you want to set one up click here"
                            buttonText: "Setup…"
                        }
                        ListElement {
                            title: "Send document via email"
                            description: "This will send the document as an attached file to your own email for later sync"
                            buttonText: "Send As Email"
                        }
                    }
                    header: KD.SubtitleDelegate {
                        hoverEnabled: false
                        down: false

                        width: ListView.view?.width

                        text: "This document has already automatically synced with your phone \"Dancepartymeister 12\". If you want to sync with another device or do further actions you can do that here"

                        icon.name: "documentinfo"
                        icon.width: LingmoUI.Units.iconSizes.large
                        icon.height: LingmoUI.Units.iconSizes.large
                    }
                    delegate: QQC2.ItemDelegate {
                        id: delegate

                        required property string title
                        required property string description
                        required property string buttonText

                        hoverEnabled: false
                        highlighted: false
                        down: false

                        width: ListView.view?.width

                        contentItem: RowLayout {
                            spacing: LingmoUI.Units.smallSpacing
                            KD.TitleSubtitle {
                                Layout.fillWidth: true
                                title: delegate.title
                                subtitle: delegate.description
                            }
                            QQC2.Button {
                                text: delegate.buttonText
                                onClicked: sheet.close()
                            }
                        }
                    }
                }
            }

            QQC2.TextArea {
                background: null
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                text: `Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec sollicitudin, lorem at semper pretium, tortor nisl pellentesque risus, eget eleifend odio ipsum ac mi. Donec justo ex, elementum vitae gravida vel, pretium ac lacus. Duis non metus ac enim viverra auctor in non nunc. Sed sit amet luctus nisi. Proin justo nulla, vehicula eget porta sit amet, aliquet vitae dolor. Mauris sed odio auctor, tempus ipsum ac, placerat enim. Ut in dolor vel ante dictum auctor.

    Praesent blandit rhoncus augue. Phasellus consequat luctus pulvinar. Pellentesque rutrum laoreet dolor, sit amet pellentesque tellus mattis sed. Sed accumsan cursus tortor. Morbi et risus dolor. Nullam facilisis ipsum justo, nec sollicitudin mi pulvinar ac. Nulla facilisi. Donec maximus turpis eget mollis laoreet. Phasellus vel mauris et est mattis auctor eget sit amet turpis. Aliquam dignissim euismod purus, eu efficitur neque fermentum eu. Suspendisse potenti. Praesent mattis ex vitae neque rutrum tincidunt. Etiam placerat leo viverra pulvinar tincidunt.

    Proin vel rutrum massa. Proin volutpat aliquet dapibus. Maecenas aliquet elit eu venenatis venenatis. Ut elementum, lacus vel auctor auctor, velit massa elementum ligula, quis elementum ex nisi aliquam mauris. Nulla facilisi. Pellentesque aliquet egestas venenatis. Donec iaculis ultrices laoreet. Vestibulum cursus rhoncus sollicitudin.

    Proin quam libero, bibendum eget sodales id, gravida quis enim. Duis fermentum libero vitae sapien hendrerit, in tincidunt tortor semper. Nullam quam nisi, feugiat sed rutrum vitae, dignissim quis risus. Ut ultricies pellentesque est, ut gravida massa convallis sed. Ut placerat dui non felis interdum, id malesuada nulla ornare. Phasellus volutpat purus placerat velit porta tristique. Donec molestie leo in turpis bibendum pharetra. Fusce fermentum diam vitae neque laoreet, sed aliquam leo sollicitudin.

    Ut facilisis massa arcu, eu suscipit ante varius sed. Morbi augue leo, mattis eu tempor vitae, condimentum sed urna. Curabitur ac blandit orci. Vestibulum quis consequat nunc. Proin imperdiet commodo imperdiet. Aenean mattis augue et imperdiet ultricies. Ut id feugiat nulla, et sollicitudin dui. Etiam scelerisque ligula ac euismod hendrerit. Integer in quam nibh. Pellentesque risus massa, porttitor quis fermentum eu, dictum varius magna. Morbi euismod bibendum lacus efficitur pretium. Phasellus elementum porttitor enim nec dictum. Morbi et augue laoreet, convallis quam quis, egestas quam.`
            }
        }
    }
}
