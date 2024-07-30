/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PM
import org.kde.kitemmodels as KItemModels
import "components"

Kirigami.Dialog {
    id: root

    property bool loading: false
    property bool showingManual: false
    property bool hasDetectedDevices: false

    // MFG:HP;MDL:ENVY 4520 series;CLS:PRINTER;DES:ENVY 4520 series;SN:TH6BN4M1390660;
    function parseDeviceId(devId: string, key: string) : var {
        if (devId === undefined) {
            return ""
        }

        // if no key, return the array
        const arr = devId.split(";")
        if (key === undefined) {
            return arr
        }

        // otherwise, return key[value]
        for (let i=0, len=arr.length; i<len; ++i) {
            const a = arr[i].split(":")
            if (a[0] === key)
                return a[1]
        }

        return ""
    }

    // Remove the ppd settings to force manual make/model driver selection
    function manualDriverSelect() {
        settings.remove("ppd-name")
        settings.add("ppd-type", PM.PPDType.Custom)
        root.setValues(settings.pending)
        close()
    }

    // Find the first direct device and network device
    // note, we're looping the decscendent filter model
    function setDeviceSelection() {
        let directNdx = -1
        let netNdx = -1
        for (let i=0, len=deviceItems.rowCount(); i<len; ++i) {
            const ndx = deviceItems.mapToSource(deviceItems.index(i,0))
            const cls = deviceItems.sourceModel.data(ndx, PM.DevicesModel.DeviceClass)
            const devId = deviceItems.sourceModel.data(ndx, PM.DevicesModel.DeviceId)
            if (cls.toString() === "direct") {
                directNdx = i
            } else if (cls.toString() === "network" && devId.length > 0) {
                netNdx = i
            }
            if (netNdx >= 0 && directNdx >= 0)
                break
        }

        // Did we actually find device, either direct or network?
        if (directNdx === -1 && netNdx === -1) {
            compLoader.sourceComponent = noDevicesComp
            hasDetectedDevices = false
            showingManual = true
            deviceItems.invalidateFilter()
        } else {
            hasDetectedDevices = true
            // by default, select direct connect printer
            deviceList.currentIndex = directNdx !== -1 ? directNdx : netNdx
            deviceList.itemAtIndex(deviceList.currentIndex).clicked()
        }

    }

    signal setValues(var values)

    title: i18nc("@title:window", "Set up a Printer Connection")

    standardButtons: Kirigami.Dialog.NoButton

    customFooterActions: [
        Kirigami.Action {
            text: showingManual
                  ? i18nc("@action:button", "Show Detected Devices")
                  : i18nc("@action:button", "Show Manual Options")
            icon.name: showingManual
                    ? "standard-connector-symbolic"
                    : "internet-services"
            visible: hasDetectedDevices
            onTriggered: {
                showingManual = !showingManual
                deviceItems.invalidateFilter()
                deviceList.currentIndex = -1
                compLoader.sourceComponent = undefined

                if (!showingManual) {
                    setDeviceSelection()
                } else {
                    compLoader.sourceComponent = chooseManualComp
                }
            }
        },
        Kirigami.Action {
            text: i18n("Refresh")
            enabled: !loading
            icon.name: "view-refresh-symbolic"
            onTriggered: {
                showingManual = false
                devices.load()
            }
        }
    ]

    footerLeadingComponent: Kirigami.UrlButton {
        text: i18n("CUPS Network Printers Help")
        url: "http://localhost:631/help/network.html"
        padding: Kirigami.Units.largeSpacing
    }

    onClosed: destroy(10)

    ConfigValues {
        id: settings
    }

    // Filter the descendants to exclude "null" deviceClass
    KItemModels.KSortFilterProxyModel {
        id: deviceItems
        sortRole: PM.DevicesModel.DeviceCategory

        // Descendants are the actual printer devices
        sourceModel: KItemModels.KDescendantsProxyModel {
            sourceModel: devices
        }

        filterRowCallback: (source_row, source_parent) => {
           const ndx = sourceModel.index(source_row, 0, source_parent)
           if (sourceModel.data(ndx, PM.DevicesModel.DeviceClass) === undefined) {
               return false
           }
           const cat = sourceModel.data(ndx, PM.DevicesModel.DeviceCategory)
           if (showingManual) {
               return cat === "Manual"
           } else {
               return cat !== "Manual"
           }

        }
    }

    // Two-level QSIM, top level is "device category" (Qt.UserRole)
    PM.DevicesModel {
        id: devices

        function load() {
            loading = true
            kcm.clearRemotePrinters()
            kcm.clearRecommendedDrivers()
            update()
        }

        Component.onCompleted: load()

        onLoaded: {
            loading = false
            setDeviceSelection()
        }
    }

    Component {
        id: noDevicesComp

        Kirigami.PlaceholderMessage {
            text: i18nc("@info:status", "Unable to automatically discover any printing devices")
            explanation: i18nc("@info:usagetip", "Choose \"Refresh\" to try again or choose a manual configuration option from the list")
            Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing * 4
        }
    }

    Component {
        id: chooseManualComp

        Kirigami.PlaceholderMessage {
            text: i18nc("@info:usagetip", "Choose a manual configuration option from the list")
            Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing * 4
        }
    }

    contentItem: RowLayout {
        spacing: 0

        QQC2.ScrollView {
            Layout.fillHeight: true
            Layout.preferredWidth: Kirigami.Units.gridUnit*13
            clip: true

            contentItem: ListView {
                id: deviceList

                QQC2.BusyIndicator {
                    id: busyInd
                    running: loading
                    anchors.centerIn: parent
                    implicitWidth: Math.floor(parent.width/2)
                    implicitHeight: implicitWidth
                }

                clip: true
                currentIndex: -1

                activeFocusOnTab: true
                keyNavigationWraps: true

                model: deviceItems

                section {
                    property: "deviceCategory"
                    delegate: Kirigami.ListSectionHeader {
                        width: ListView.view.width
                        required property string section
                        label: section
                    }
                }

                delegate: Kirigami.SubtitleDelegate {
                    width: ListView.view.width
                    visible: deviceClass !== undefined

                    text: deviceInfo.replace("Internet Printing Protocol", "IPP")
                    subtitle: deviceMakeModel.replace("Unknown", "")

                    icon.name: deviceId.length === 0
                               ? "internet-services"
                               : "printer-symbolic"

                    highlighted: ListView.view.currentIndex === index
                    onClicked: {
                        ListView.view.currentIndex = index
                        compLoader.selector = ""
                        compLoader.info = ""
                        settings.clear()

                        if (deviceUri && deviceUri.length > 0) {
                            compLoader.info = deviceInfo
                            if (deviceId && deviceId.length > 0) {
                                if (deviceClass === "file") {
                                    compLoader.selector = deviceUri.replace(/:\//g,'')
                                } else {
                                    // a printer device
                                    settings.set({"device-id": deviceId
                                                    , "device-uri": deviceUri
                                                    , "device-uris": deviceUris
                                                    , "device-class": deviceClass
                                                    , "device-desc": deviceDescription
                                                    , "printer-info": deviceInfo
                                                    , "printer-make": parseDeviceId(deviceId, "MFG")
                                                    , "printer-model": parseDeviceId(deviceId, "MDL")
                                                    , "printer-make-and-model": deviceMakeModel
                                                    , "printer-location": deviceLocation
                                                    , "ppd-type": PM.PPDType.Custom
                                                 })
                                    compLoader.selector = deviceClass
                                }

                            } else {
                                // a category item
                                compLoader.selector = deviceUri
                            }
                        }
                    }
                }
            }
        }

        Kirigami.Separator {
            Layout.fillHeight: true
            width: 1
        }

        Loader {
            id: compLoader
            active: !loading

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Kirigami.Units.largeSpacing

            property string selector: ""
            property string info: ""

            onSelectorChanged: {
                switch (selector) {
                case "other":
                case "ipp":
                case "ipps":
                case "http":
                case "https":
                case "scsi":
                case "cups-brf":
                    source = "components/ManualUri.qml"
                    break
                case "network":
                    source = "components/Network.qml"
                    break
                case "direct":
                    source = "components/Direct.qml"
                    break
                case "lpd":
                    source = "components/Lpd.qml"
                    break
                case "socket":
                    source = "components/Socket.qml"
                    break
                case "smb":
                case "serial":
                default:
                    source = "components/NotAvailable.qml"
                }
            }
        }

    }

}
