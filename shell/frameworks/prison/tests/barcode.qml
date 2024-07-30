/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import org.kde.prison 1.0 as Prison
Rectangle {
    width: 640
    height: 320
    color: "lightsteelblue"
    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: contentEdit
                Layout.fillWidth: true
                text: "KF5::Prison - The KDE barcode generation framework."
            }
            ComboBox {
                id: typeCombobox
                model: [ "QRCode", "DataMatrix", "Aztec", "Code39", "Code93", "Code128", "PDF417", "EAN13" ]
                currentIndex: 3
            }
            Button {
		text: "undef"
                onClicked: barcode.barcodeType = undefined
            }
        }

        Prison.Barcode {
            id: barcode
            Layout.fillWidth: true
            Layout.fillHeight: true
            content: contentEdit.text
            barcodeType: typeCombobox.currentIndex
//             foregroundColor: "red"
//             backgroundColor: "green"
        }

        RowLayout {
            Label {
                text: "1D: " + (barcode.dimensions == Prison.Barcode.OneDimension)
            }
            Label {
                text: "2D: " + (barcode.dimensions == 2)
            }
            Label {
                text: "Min size: " + barcode.minimumWidth + "x" + barcode.minimumHeight
            }
        }
        Prison.Barcode {
            id: nullbarcode
            Layout.fillWidth: true
            Layout.fillHeight: true
            content: contentEdit.text
	    Component.onCompleted: {
               console.log(nullbarcode.barcodeType)
            }
        }
    }
}
