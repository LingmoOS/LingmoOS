// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

import org.kde.kcmutils as KCM
import QtQuick 2.14
import QtQml.Models 2.14
import QtQuick.Layouts 1.14
import SMART 1.0 as SMART
import org.kde.lingmoui 2.12 as LingmoUI
import QtQuick.Controls 2.14

KCM.SimpleKCM {
    id: root
    implicitWidth: LingmoUI.Units.gridUnit * 28
    implicitHeight: LingmoUI.Units.gridUnit * 20

    SMART.ServiceRunner {
        id: partitionManagerRunner
        name: "org.kde.partitionmanager"
    }

    SMART.ServiceRunner {
        id: kupRunner
        name: "kcm_kup"
    }

    actions: [
        LingmoUI.Action {
            visible: partitionManagerRunner.canRun
            text: i18nc("@action/button action button to start partition manager", "Open Partition Manager")
            icon.name: "partitionmanager"
            onTriggered: partitionManagerRunner.run()
        },
        LingmoUI.Action {
            visible: kupRunner.canRun
            text: i18nc("@action/button action button to start backup program", "Perform Backup")
            icon.name: "kup"
            onTriggered: kupRunner.run()
        }
    ]

    LingmoUI.CardsListView {
        id: listView
        width: 110
        height: 160
        model: SMART.DeviceModel { id: deviceModel }

        LingmoUI.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)

            opacity: !deviceModel.valid
            Behavior on opacity { NumberAnimation { duration: LingmoUI.Units.longDuration; easing.type: Easing.InOutQuad } }

            icon.name: "messagebox_warning"
            text: i18nc("@info/status", "Unable to obtain data. KDED is not running.")
        }

        LingmoUI.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)

            opacity: deviceModel.valid && !deviceModel.waiting && parent.count <= 0
            Behavior on opacity { NumberAnimation { duration: LingmoUI.Units.longDuration; easing.type: Easing.InOutQuad } }

            icon.name: "edit-none"
            text: i18nc("@info/status", "No S.M.A.R.T. devices found.")
        }

        delegate: LingmoUI.Card {
            banner.title: "%1 (%2)".arg(product).arg(path)
            banner.titleIcon: {
                if (failed) {
                    return "data-warning"
                }
                if (instabilities.length !== 0) {
                    return "data-information"
                }
                return ""
            }

            contentItem: Label {
                anchors.fill: parent
                wrapMode: Text.Wrap
                text: {
                    if (failed) {
                        return i18nc("@info",
                            "The SMART system of this device is reporting problems. This may be a sign of imminent device failure or data reliability being compromised. " +
                            "Back up your data and replace this drive as soon as possible to avoid losing any data.")
                    }
                    if (instabilities.length !== 0) {
                        var items = instabilities.map(item => "<li>%1</li>".arg(item))
                        return i18nc("@info %1 is a bunch of <li> with the strings from instabilities.cpp",
                            "<p>The SMART firmware is not reporting a failure, but there are early signs of malfunction. " +
                            "This might not point at imminent device failure but requires longer term analysis. " +
                            "Back up your data and contact the manufacturer of this disk, or replace it preemptively just to be safe.</p>" +
                            "<ul>%1</ul>", items.join(''))
                    }
                    return i18nc("@info",
                            "This device appears to be working as expected.")
                }
            }

            actions: [
                LingmoUI.Action {
                    text: ignore
                          ? i18nc("@action/button action button to monitor a device for smart failure", "Monitor")
                          : i18nc("@action/button action button to ignore smart failures for a device", "Ignore")
                    icon.name: ignore ? "view-visible" : "view-hidden"
                    onTriggered: {
                        model.ignore = !ignore
                    }
                },
                LingmoUI.Action {
                    visible: model.advancedReport !== ''
                    text: i18nc("@action/button show detailed smart report", "Show Details")
                    icon.name: "showinfo-symbolic"
                    onTriggered: {
                        // NB: push daftly hardcodes ui/ as prefix....
                        kcm.push("ReportPage.qml", {title: product, text: advancedReport})
                    }
                }
            ]
        }
    }
}
