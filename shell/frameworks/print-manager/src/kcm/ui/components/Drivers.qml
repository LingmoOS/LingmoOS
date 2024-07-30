/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PM

ColumnLayout {
    id: root
    spacing: Kirigami.Units.largeSpacing

    readonly property bool ippCapable: kcm.isIPPCapable(settings.value("device-uri"))
    readonly property alias busy: kcmConn.loading

    // one of the recommended drivers was selected
    function selectDriver(driverMap) {
        settings.set(driverMap)
        setValues(settings.pending)
        close()
    }

    function load(devid, makeModel, uri) {
        if (busy) {
            return
        }

        kcmConn.loading = true
        kcm.getRecommendedDrivers(devid, makeModel, uri)
    }

    // Fallback msg with the option to select the driver manually
    Kirigami.InlineMessage {
        id: fallbackMsg

        text: xi18nc("@info:status", "Unable to locate recommended drivers.  Click <interface>Refresh</interface> to try again or choose a driver manually.")
        showCloseButton: false
        Layout.fillWidth: true

        actions: [
            Kirigami.Action {
                icon.name: "favorites-symbolic"
                visible: ippCapable
                text: i18nc("@action:button", "Choose Driverless…")
                onTriggered: selectDriver({"ppd-name": "everywhere", "ppd-type": PM.PPDType.Auto})
            },
            Kirigami.Action {
                icon.name: "document-edit-symbolic"
                text: i18nc("@action:button", "Choose Driver…")
                onTriggered: manualDriverSelect()
            }
        ]
    }

    Connections {
        id: kcmConn
        target: kcm

        property bool loading: false

        function onRecommendedDriversLoaded() {
            loading = false

            // For whatever reason, we failed to get recommended drivers
            if (kcm.recommendedDrivers.length === 0) {
                fallbackMsg.visible = true
            }
        }
    }

    QQC2.BusyIndicator {
        running: kcmConn.loading
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        implicitWidth: Kirigami.Units.gridUnit * 6
        implicitHeight: Kirigami.Units.gridUnit * 6
    }

    QQC2.Button {
        id: recmAction
        Layout.alignment: Qt.AlignHCenter
        visible: !fallbackMsg.visible
        enabled: !busy
        icon.name: "dialog-ok-symbolic"
        text: i18nc("@action:button", "Select Recommended Driver")

        onClicked: selectDriver(kcm.recommendedDrivers[recmlist.currentIndex])

        QQC2.ToolTip {
            text: i18nc("@info:tooltip", "Recommended drivers are based on printer make/model and connection type")
        }
    }

    QQC2.ScrollView {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: !fallbackMsg.visible

        Component.onCompleted: {
            if (background) {
                background.visible = true
            }
        }

        contentItem: ListView {
            id: recmlist

            activeFocusOnTab: true
            keyNavigationWraps: true

            KeyNavigation.backtab: root.parent
            Keys.onUpPressed: event => {
                if (currentIndex === 0) {
                    currentIndex = -1;
                }
                event.accepted = false;
            }

            model: kcm.recommendedDrivers

            delegate: Kirigami.SubtitleDelegate {
                width: ListView.view.width

                text: {
                    const ppdname = modelData["ppd-name"]
                    if (ppdname.includes("driverless")) {
                        return i18nc("@label:listitem", "Driverless (%1)", modelData.match)
                    } else if (ppdname.includes("ppd")) {
                        return i18nc("@label:listitem", "PPD File (%1)", modelData.match)
                    } else {
                        return modelData.match
                    }
                }
                subtitle: modelData["ppd-name"]
                icon.name: {
                    if (ippCapable) {
                        return modelData["ppd-name"].startsWith("driverless")
                               ? "favorites-symbolic"
                               : "dialog-question-symbolic"
                    } else {
                        return modelData.match.startsWith("exact")
                               ? "favorites-symbolic"
                               : "dialog-question-symbolic"
                    }
                }

                highlighted: index === ListView.view.currentIndex
                onClicked: ListView.view.currentIndex = index
            }
        }
    }

}
