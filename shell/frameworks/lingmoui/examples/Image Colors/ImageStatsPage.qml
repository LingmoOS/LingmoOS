/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

import "components" as Components

LingmoUI.ScrollablePage {
    id: root

    required property Image image
    required property LingmoUI.ImageColors imagePalette
    required property /*ImageProviders.ImageData*/ var imageData
    required property Item overlay

    header: QQC2.ToolBar {
        contentItem: LingmoUI.ActionToolBar {
            alignment: Qt.AlignHCenter
            actions: [
                LingmoUI.Action {
                    text: "Update Palette"
                    icon.name: "view-refresh-symbolic"
                    displayHint: LingmoUI.DisplayHint.KeepVisible
                    onTriggered: source => {
                        imagePalette.update();
                    }
                }
            ]
        }
    }

    ColumnLayout {
        spacing: LingmoUI.Units.gridUnit

        Components.Section {
            title: "Image"

            LingmoUI.UrlButton {
                url: root.imageData?.image_page ?? ""
                text: root.imageData?.image_description ?? ""
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        Components.Section {
            title: "Author"

            LingmoUI.UrlButton {
                url: root.imageData?.author_url ?? ""
                text: root.imageData?.author_name ?? ""
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        Components.Section {
            title: "Controls"

            QQC2.CheckBox {
                id: showOverlayCheckbox
                Layout.fillWidth: true
                text: "Show Overlay"
                checked: root.overlay?.visible ?? false
                onToggled: {
                    const overlay = root.overlay;
                    if (overlay) {
                        overlay.visible = checked;
                    }
                }
            }
        }

        Components.Section {
            title: "Image Palette"

            Components.PaletteTable {
                swatches: imagePalette.palette
            }
        }
    }
}
