/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as QtDialogs
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

import "components" as Components
import "ImageProviders.mjs" as ImageProviders

QQC2.Page {
    id: root

    readonly property alias image: image

    readonly property LingmoUI.ImageColors imagePalette: LingmoUI.ImageColors {
        id: imagePalette
        source: image
    }

    readonly property /*ImageProviders.ImageData*/ var imageData: __imageData

    readonly property Item overlay: overlay

    property /*XMLHttpRequest*/var __xhr: null
    property /*ImageProviders.ImageData*/ var __imageData: null

    function __abort(): void {
        if (__xhr !== null) {
            __xhr.abort();
            __xhr = null;
        }
    }

    function randomizeSource(): void {
        __abort();
        const provider = new ImageProviders.UnsplashProvider();
        __xhr = provider.getRandom(imageData => {
            __xhr = null;
            __imageData = imageData;
        });
    }

    function openChooserDialog(): void {
        fileDialog.open();
    }

    function setImageSource(fileUrl: url): void {
        __abort();
        const provider = new ImageProviders.FileProvider(fileUrl);
        __imageData = provider.imageData();
    }

    Component.onCompleted: {
        randomizeSource();
    }

    header: QQC2.ToolBar {
        contentItem: LingmoUI.ActionToolBar {
            alignment: Qt.AlignHCenter
            actions: [
                LingmoUI.Action {
                    icon.name: "shuffle-symbolic"
                    text: "Random Image"
                    displayHint: LingmoUI.DisplayHint.KeepVisible
                    onTriggered: source => {
                        root.randomizeSource();
                    }
                },
                LingmoUI.Action {
                    icon.name: "insert-image-symbolic"
                    text: "Open Image…"
                    displayHint: LingmoUI.DisplayHint.KeepVisible
                    onTriggered: source => {
                        root.openChooserDialog();
                    }
                }
            ]
        }
    }

    QtDialogs.FileDialog {
        id: fileDialog

        title: "Open Image"
        nameFilters: "Media Files (*.jpg *.png *.svg)"
        fileMode: QtDialogs.FileDialog.OpenFile
        options: QtDialogs.FileDialog.ReadOnly

        onAccepted: {
            root.setImageSource(selectedFile);
        }
    }

    Image {
        id: image

        anchors.fill: parent
        source: root.__imageData?.image_url ?? "";
        fillMode: Image.PreserveAspectFit

        onStatusChanged: {
            imagePalette.update();
        }
    }

    QQC2.Pane {
        id: overlay

        anchors.centerIn: parent

        width: LingmoUI.Units.gridUnit * 15

        LingmoUI.Theme.backgroundColor: imagePalette.background
        LingmoUI.Theme.textColor: imagePalette.foreground
        LingmoUI.Theme.highlightColor: imagePalette.highlight

        background: Rectangle {
            color: LingmoUI.Theme.backgroundColor
            border.width: 1
            border.color: LingmoUI.Theme.textColor
            radius: LingmoUI.Units.cornerRadius
            opacity: 0.8
        }

        contentItem: ColumnLayout {
            spacing: LingmoUI.Units.largeSpacing

            QQC2.Label {
                text: "Highlight Color:\nLorem Ipsum dolor sit amet"
                color: LingmoUI.Theme.highlightColor
                wrapMode: Text.Wrap
            }

            Components.ColorWell {
                Layout.fillWidth: true
                color: LingmoUI.Theme.highlightColor
            }

            LingmoUI.Separator {
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: "Text Color:\nLorem Ipsum dolor sit amet"
                color: LingmoUI.Theme.textColor
                wrapMode: Text.Wrap
            }

            Components.ColorWell {
                Layout.fillWidth: true
                color: LingmoUI.Theme.textColor
            }

            LingmoUI.Separator {
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: "Controls with inherited theme"
                color: LingmoUI.Theme.textColor
                wrapMode: Text.Wrap
            }


            RowLayout {
                spacing: LingmoUI.Units.largeSpacing

                QQC2.TextField {
                    LingmoUI.Theme.inherit: true
                    Layout.fillWidth: true
                    text: "text"
                }

                QQC2.Button {
                    LingmoUI.Theme.inherit: true
                    text: "Ok"
                }
            }
        }
    }
}
