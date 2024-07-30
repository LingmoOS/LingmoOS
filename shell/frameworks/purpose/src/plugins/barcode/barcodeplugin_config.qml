/*
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.prison as Prison

ColumnLayout {
    id: root

    property list<string> urls

    spacing: Kirigami.Units.smallSpacing

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        Layout.preferredHeight: Kirigami.Units.gridUnit * 10

        Prison.Barcode {
            id: barcodeItem
            readonly property bool valid: implicitWidth > 0 && implicitHeight > 0 && implicitWidth <= width && implicitHeight <= height
            anchors.fill: parent
            barcodeType: Prison.Barcode.QRCode
            // Cannot set visible to false as we need it to re-render when changing its size
            opacity: valid ? 1 : 0
            content: textField.text
        }

        QQC2.Label {
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: {
                if (textField.length === 0) {
                    return i18nd("purpose6_barcode", "Type a URL or some text to generate a QR code");
                } else if (textField.length > 0 && barcodeItem.implicitWidth === 0 && barcodeItem.implicitHeight === 0) {
                    return i18nd("purpose6_barcode", "Creating QR code failed");
                } else if (textField.length > 0 && (barcodeItem.implicitWidth > barcodeItem.width || barcodeItem.implicitHeight > barcodeItem.height)) {
                    return i18nd("purpose6_barcode", "The QR code is too large to be displayed");
                } else {
                    return "";
                }
            }
            wrapMode: Text.Wrap
        }
    }

    QQC2.TextField {
        id: textField
        Layout.fillWidth: true
        text: root.urls[0]
        // Random limit so it doesn't get too large
        maximumLength: 250
        placeholderText: i18nd("purpose6_barcode", "Type a URL or some text…")
        Component.onCompleted: forceActiveFocus()
    }
}
