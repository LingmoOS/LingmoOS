/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * Device setup for LPD/LPR printer devices (Legacy)
 *
 * Uses the base URI component and adds the LPD queue
 * field.
*/
BaseDevice {
    id: lpdDevice
    title: compLoader.info
    subtitle: i18nc("@title:group", "Line Printer Daemon (LPD) Protocol")
    helpText: i18nc("@info:usagetip", "Enter the address of the LPD device")
    showUriSearch: false

    scheme: compLoader.selector + "://"

    showAddressExamples: true
    addressExamples: [
        "lpd://ip-addr/queue",
        "lpd://ip-addr/queue?format=l",
        "lpd://ip-addr/queue?format=l&reserve=rfc1179",
    ]

    contentItem: ColumnLayout {
        width: lpdDevice.width
        // use large here to match Base
        spacing: Kirigami.Units.largeSpacing

        Component.onCompleted: {
            const url = lpdDevice.getUrl(settings.value("device-uri"))
            if (url) {
                lpdDevice.uriText = scheme + url.hostname
                queue.text = url.pathname + url.search
            } else {
                lpdDevice.uriText = scheme
            }
        }

        Connections {
            target: lpdDevice
            function onAddressExampleSelected(address) {
                const url = lpdDevice.getUrl(address)
                lpdDevice.uriText = scheme + url.hostname
                queue.text = url.pathname + url.search
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18nc("@label:textbox", "Queue:")
            }

            QQC2.TextField {
                id: queue
                placeholderText: i18nc("@info:placeholder The LPD device queue name", "LPD queue name")
                validator: RegularExpressionValidator { regularExpression: /[^/#\\ ]*/ }
                Layout.fillWidth: true
            }
        }

        QQC2.Button {
            text: i18nc("@action:button", "Continue with Address…")
            icon.name: "dialog-ok-symbolic"
            enabled: lpdDevice.uriText.length > 0
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                let deviceAddress = lpdDevice.uriText + "/" + queue.text.replace(/\//g, "")
                if (!deviceAddress.startsWith(scheme)) {
                    deviceAddress = scheme + deviceAddress
                }
                // validate url
                let url = lpdDevice.getUrl(deviceAddress)
                if (url) {
                    settings.set({"device-uri": url.href
                                 , "printer-info": "LPD Printer"})
                    manualDriverSelect()
                } else {
                    lpdDevice.setError(i18nc("@info:status", "Invalid LPD URL: %1", deviceAddress))
                }
            }
        }
    }
}
