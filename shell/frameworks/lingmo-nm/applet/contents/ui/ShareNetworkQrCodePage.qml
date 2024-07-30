/*
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Layouts 1.2

import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.prison 1.0 as Prison

ColumnLayout {
    id: page

    property string ssid
    property alias content: barcode.content

    spacing: LingmoUI.Units.smallSpacing

    LingmoComponents3.Label {
        Layout.topMargin: page.spacing
        Layout.alignment: Qt.AlignHCenter
        Layout.minimumWidth: barcode.height
        Layout.maximumWidth: barcode.width - page.spacing * 2
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        textFormat: Text.PlainText
        text: i18n("Scan this QR code with another device to connect to the \"%1\" network.", page.ssid)
    }

    Prison.Barcode {
        id: barcode
        Layout.fillWidth: true
        Layout.fillHeight: true
        barcodeType: Prison.Barcode.QRCode

        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.CopyAction

        HoverHandler {
            enabled: barcode.valid
            cursorShape: Qt.OpenHandCursor
        }

        DragHandler {
            id: dragHandler
            enabled: barcode.valid
            cursorShape: Qt.ClosedHandCursor
            target: null

            onActiveChanged: {
                if (active) {
                    barcode.grabToImage((result) => {
                        barcode.Drag.mimeData = {
                            "image/png": result.image,
                        };
                        barcode.Drag.active = dragHandler.active;
                    });
                } else {
                    barcode.Drag.active = false;
                }
            }
        }
    }
}
