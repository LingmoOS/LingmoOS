/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 
import QtQuick.Layouts 
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.printmanager as PM

/** Note: CUPS has nomenclature for groups of printers called a "Class".
* For user-facing elements, we will use the term "Group".
*
* For CUPS auth and global (print server) settings we will check if
* server settings are loaded before each maint action.  Getting server
* settings forces CUPS auth.
*/

KCM.ScrollViewKCM {
    id: root
    headerPaddingEnabled: false

    function newPrinter(isPrinter = true, addlObj, ppdObj) {
        const obj = {
            printerName: isPrinter ? i18n("new_queue") : i18n("new_group")
            , isClass: !isPrinter
            , kind: ""
            , remote: false
            , isDefault: false
            , isShared: false
            , isAcceptingJobs: true
            , iconName: isPrinter ? "printer" : "folder-print"
            , memberNames: []
            , commands: []
        }

        if (addlObj !== undefined && typeof addlObj === "object") {
            Object.assign(obj, addlObj)
        }

        kcm.push("PrinterSettings.qml"
                , {modelData: Object.assign({}, obj)
                , ppd: Object.assign({}, ppdObj)
                , addMode: true
                , printerModel: pmModel
                , ppdModel: ppdModel
                })
    }

    function checkServerSettings() {
        if (!kcm.serverSettingsLoaded) {
            // Calls CUPS Auth
            kcm.getServerSettings()
        }
    }

    header: Kirigami.InlineMessage {
        id: scpMessage
        showCloseButton: true
        type: Kirigami.MessageType.Warning
        position: Kirigami.InlineMessage.Position.Header
        visible: !kcm.isSCPAvailable()
        text: {
            if (PM.SCPInstaller === undefined) {
                xi18nc("@info:usagetip", "A printer support package that provides convenience features does not appear to be installed.<nl/><nl/>Because this distro does not include PackageKit, we cannot provide an install option, so you will have to use your package manager to install the <command>system-config-printer</command> package manually.")
            } else {
                i18nc("@info:usagetip", "A printer support package that provides convenience features does not appear to be installed.")
            }
        }

        actions: [
            Kirigami.Action {
                text: i18nc("@action:button Install printer setup helper package", "Install It")
                icon.name: "install-symbolic"
                visible: PM.SCPInstaller !== undefined
                onTriggered: scpLoader.active = true
            }
        ]
    }

    /**
    * The SCPInstaller type is registered into the PrinterManager namespace when
    * the kcm is compiled with a found PackageKit and the SCP_Install option is
    * enabled.  If the SCPInstaller is not registered, it will be undefined.
    *
    * If registered, use the loader to create the component which will install SCP
    * with PackageKit.  See "Install It" action above.
    */
    Loader {
        id: scpLoader
        active: false
        anchors.centerIn: parent
        source: "InstallDialog.qml"

        onActiveChanged: {
            if (active) {
                item.parent = root
            }
        }
    }

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button The thing being added is a printer", "Add…")
            Accessible.name: i18nc("@action:button", "Add Printer…")
            icon.name: "list-add-symbolic"
            onTriggered: {
                checkServerSettings()
                const dlg = findComp.createObject(root)
                dlg.open()
            }
        }
        , Kirigami.Action {
            text: i18nc("@action:button", "Add Group…")
            icon.name: "folder-print-symbolic"
            visible: list.count > 1
            onTriggered: {
                checkServerSettings()
                newPrinter(false)
            }
        }
        , Kirigami.Action {
            text: i18nc("@action:button", "Configure Print Server…")
            icon.name: "configure-symbolic"
            onTriggered: {
                const comp = Qt.createComponent("Global.qml")
                const dlg = comp.createObject(root)
                dlg.open()
            }
        }
    ]

    Connections {
        target: kcm

        // when a successful save is done
        function onSaveDone() {
            kcm.pop()
        }

        // when a printer/class is removed
        function onRemoveDone() {
            kcm.pop()
        }

    }

    Component {
        id: findComp

        FindPrinter {
            anchors.centerIn: parent
            implicitWidth: Math.ceil(parent.width*.90)
            implicitHeight: Math.ceil(parent.height*.90)

            // Selected printer and/or driver
            // ppd-name contains the driver file name
            onSetValues: configMap => {
                const cfgObj = {info: configMap["printer-info"]
                             , printerUri: configMap["device-uri"]
                             , location: configMap["printer-location"]
                             , "ppd-type": configMap["ppd-type"]
                             , "ppd-name": configMap["ppd-name"] ?? ""}

                if (configMap.hasOwnProperty("printer-model")) {
                    cfgObj.printerName = configMap["printer-model"].replace(/ /g, "_")
                }

                // Set the PPD attrs
                const ppdObj = {make: configMap["printer-make"]
                             , makeModel: configMap["printer-make-and-model"]
                             , type: configMap["ppd-type"]
                             , file: configMap["ppd-name"] ?? ""}

                // if we have device file
                // strip out the base file name
                if (ppdObj.file) {
                     cfgObj.kind = ppdObj.makeModel
                     const i = ppdObj.file.lastIndexOf('/')
                     if (i !== -1) {
                         ppdObj.pcfile = ppdObj.file.slice(-(ppdObj.file.length-i-1))
                     } else {
                         ppdObj.pcfile = ppdObj.file
                     }
                } else {
                    cfgObj.kind = ""
                    ppdObj.pcfile = ""
                }

                root.newPrinter(true, cfgObj, ppdObj)

            }
        }
    }

    PM.PrinterModel {
        id: pmModel
    }

    // Not heavy, but slow (timeout?), loads on-demand
    PM.PPDModel {
        id: ppdModel
    }

    view: ListView {
        id: list
        clip: true
        spacing: Kirigami.Units.largeSpacing

        Keys.onPressed: event => {
            if (event.key === Qt.Key_Home) {
                positionViewAtBeginning();
                currentIndex = 0;
                event.accepted = true;
            }
            else if (event.key === Qt.Key_End) {
                positionViewAtEnd();
                currentIndex = count - 1;
                event.accepted = true;
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: list.count === 0
            icon.name: "printer"
            text: i18nc("@info:status", "No printers are currently set up")
            explanation: xi18nc("@info:usagetip", "Click <interface>Add Printer…</interface> to set up a new printer on this computer")
        }
        
        // If there is a mix of printers and classes (groups), then show
        // the section header
        section {
            property: pmModel.printersOnly ? "" : "isClass"
            delegate: Kirigami.ListSectionHeader {
                width: ListView.view.width
                required property bool section
                label: !section ? i18n("Printers") : i18n("Printer Groups")
            }
        }

        model: KItemModels.KSortFilterProxyModel {
            sourceModel: pmModel
            sortRoleName: "isClass"
        }

        delegate: QQC2.ItemDelegate {
            width: ListView.view.width

            hoverEnabled: false
            highlighted: false
            down: false

            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.SubtitleDelegate {
                    Layout.fillWidth: true
                    text: model.info
                          + (model.location && pmModel.displayLocationHint
                             ? " (%1)".arg(model.location)
                             : "")
                    subtitle: model.stateMessage
                    icon.name: model.remote
                            ? "folder-network-symbolic"
                            : (model.isClass ? "folder-print" : model.iconName)

                    font.bold: list.count > 1 & model.isDefault

                    hoverEnabled: false
                    highlighted: false
                    down: false
                }

                QQC2.ToolButton {
                    text: i18nc("@action:button", "Configure…")
                    icon.name: "configure-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    Layout.alignment: Qt.AlignRight|Qt.AlignVCenter

                    onClicked: {
                        checkServerSettings()
                        kcm.push("PrinterSettings.qml"
                                        , { modelData: model
                                        , addMode: false
                                        , printerModel: pmModel
                                        , ppdModel: ppdModel
                                        })
                    }

                    QQC2.ToolTip {
                        text: parent.text
                    }
                }

                QQC2.ToolButton {
                    text: i18nc("@action:button", "Open Print Queue…")
                    icon.name: "view-list-details-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    Layout.alignment: Qt.AlignRight|Qt.AlignVCenter

                    onClicked: PM.ProcessRunner.openPrintQueue(model.printerName)

                    QQC2.ToolTip {
                        text: parent.text
                    }
                }

                QQC2.ToolButton {
                    icon.name: isPaused
                               ? "media-playback-start-symbolic"
                               : "media-playback-pause-symbolic"
                    text: isPaused
                          ? i18nc("@action:button Resume printing", "Resume")
                          : i18nc("@action:button Pause printing", "Pause")

                    Layout.alignment: Qt.AlignRight|Qt.AlignVCenter

                    onClicked: {
                        if (isPaused) {
                            kcm.resumePrinter(model.printerName);
                        } else {
                            kcm.pausePrinter(model.printerName);
                        }
                    }

                    QQC2.ToolTip {
                        text: isPaused
                              ? i18nc("@info:tooltip", "Resume printing")
                              : i18nc("@info:tooltip", "Pause printing")
                    }
                }
            }
        }
    }

}
