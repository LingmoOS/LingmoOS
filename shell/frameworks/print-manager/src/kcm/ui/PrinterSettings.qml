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

KCM.AbstractKCM {
    id: root
    headerPaddingEnabled: false

    // Add mode means adding a new printer/group
    property bool addMode: false
    property var modelData
    // Printer ppd attributes
    property var ppd

    property PM.PrinterModel printerModel
    property PM.PPDModel ppdModel

    function openMakeModelDlg() {
        const dlg = mmComp.createObject(root)
        dlg.open()
    }

    function openFindPrinterDlg() {
            const dlg = newComp.createObject(root)
            dlg.open()
    }

    function printerNameIsUnique(name: string) : bool {
        for (let i=0, len=printerModel.rowCount(); i<len; ++i) {
            if (printerModel.data(printerModel.index(i,0), PM.PrinterModel.DestName).toString() === name)
                return false
        }
        return true
    }

    title: {
        if (addMode) {
            return modelData.isClass
                    ? i18nc("@title:window", "Add Group")
                    : i18nc("@title:window", "Add Printer")
        } else {
            const locStr = modelData.location && printerModel.displayLocationHint
                            ? " (%1)".arg(modelData.location)
                            : ""
            return modelData.info + locStr
        }
    }

    actions: [
        Kirigami.Action {
            text: i18n("Configure…")
            icon.name: "configure-symbolic"
            visible: !addMode
            onTriggered: PM.ProcessRunner.configurePrinter(modelData.printerName)
        }, Kirigami.Action {
            text: i18n("Remove")
            icon.name: "edit-delete-remove-symbolic"
            visible: !addMode
            onTriggered: removeLoader.active = true
        }
    ]

    header: BannerWithTimer {
        id: msgBanner
        position: Kirigami.InlineMessage.Position.Header
    }

    footer: RowLayout {
        Layout.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.UrlButton {
            text: i18nc("@action:button", "CUPS Printers Overview Help")
            url: "http://localhost:631/help/overview.html"
            padding: Kirigami.Units.largeSpacing
        }

        Kirigami.UrlButton {
            text: i18nc("@action:button", "Printer/Device Admin Page")
            visible: !modelData.isClass && url !== ""
            url: {
                try {
                    const url = new URL(devUri.text)
                    if (url.hostname.length > 0) {
                        return `http://${url.hostname}`
                    }
                } catch(e) {
                }
                return ""
            }
        }

        Item { Layout.fillWidth: true }

        QQC2.Button {
            text: addMode
                  ? i18nc("@action:button Add printer", "Add")
                  : i18nc("@action:button Apply changes", "Apply")
            icon.name: "dialog-ok-apply"
            enabled: config.hasPending

            onClicked: {
                if (addMode) {
                    if (queueName.text.length === 0) {
                        queueName.focus = true
                        msgBanner.text = i18nc("@info:status", "Queue name is required. Enter a unique queue name.")
                        msgBanner.visible = true
                        return
                    } else if (!printerNameIsUnique(queueName.text)) {
                        queueName.focus = true
                        msgBanner.text = i18nc("@info:status", "Queue name must be unique. Enter a unique queue name.")
                        msgBanner.visible = true
                        return
                    }

                    if (!modelData.isClass) {
                        if (driver.text.length === 0) {
                            driverSelect.focus = true
                            msgBanner.text = i18nc("@info:status", "Make/Model is required. Choose \"Select\" to pick Make/Model")
                            msgBanner.visible = true
                            return
                        }
                    }

                    config.add("add", true)
                }

                config.hasPending = false
                msgBanner.text = i18nc("@info:status", "Configuring %1, please wait….", !modelData.isClass ? "printer" : "group")
                msgBanner.showCloseButton = false
                msgBanner.type = Kirigami.MessageType.Positive
                msgBanner.visible = true
                kcm.savePrinter(queueName.text, config.pending, modelData.isClass)
            }
        }
    }

    Component.onCompleted: {
        if (!modelData.isClass) {
            if (addMode) {
                config.set({"ppd-name": modelData["ppd-name"]
                           , "ppd-type": modelData["ppd-type"]})

                // there is no ppd info, so show make/model selection dialog initially
                if (config.value("ppd-name") === "") {
                    openMakeModelDlg()
                }
            } else {
                ppd = kcm.getPrinterPPD(modelData.printerName)
            }
        }
    }

    // Each item stores its corresponding CUPS field name in objectName
    // This is then used to generate the "pending" changes map
    ConfigValues {
        id: config
    }

    Connections {
        id: kcmConn
        target: kcm

        property bool removing: false

        function onRequestError(errorMessage) {
            msgBanner.reset()
            if (removing) {
                removing = false
                msgBanner.text = i18n("Failed to remove the printer: %1", errorMessage)
            } else {
                msgBanner.text = errorMessage
            }
            msgBanner.visible = true
            config.clear()
        }

    }

    Loader {
        id: removeLoader
        active: false

        sourceComponent: Kirigami.PromptDialog {
            id: prompt
            parent: root

            dialogType: Kirigami.PromptDialog.Warning

            Component.onCompleted: open()
            onClosed: removeLoader.active = false

            title: modelData.isClass ? i18nc("@title:window", "Remove Group?")
                                     : i18nc("@title:window", "Remove Printer?")
            subtitle: i18nc("@info %1 is the name of a printer or printer group",
                            "'%1' will be removed.",
                            modelData.info)

            standardButtons: Kirigami.Dialog.NoButton

            customFooterActions: [
                Kirigami.Action {
                    text: modelData.isClass ? i18nc("@action:button", "Remove Group")
                                            : i18nc("@action:button", "Remove Printer")
                    icon.name: "edit-delete-remove-symbolic"
                    onTriggered: {
                        kcmConn.removing = true
                        kcm.removePrinter(modelData.printerName)
                        close()
                    }
                },
                Kirigami.Action {
                    text: i18n("Cancel")
                    icon.name: "dialog-cancel-symbolic"
                    onTriggered: close()
                }
            ]
        }

    }

    Component {
        id: newComp

        FindPrinter {
            anchors.centerIn: parent
            implicitWidth: Math.ceil(parent.width*.90)
            implicitHeight: Math.ceil(parent.height*.90)

            // Selected printer and/or driver
            // ppd-name contains the driver file
            onSetValues: configMap => {
                // Set the text entry items
                if (configMap.hasOwnProperty("printer-model")) {
                    queueName.text = configMap["printer-model"].replace(/ /g, "_")
                }
                queueInfo.text  = configMap[queueInfo.objectName]
                devUri.text     = configMap[devUri.objectName]
                location.text   = configMap[location.objectName]
                driver.text     = configMap["printer-make-and-model"]

                // Initialize the config map
                config.set(configMap)
                config.clean()

                // Set the PPD attrs
                ppd.make      = configMap["printer-make"]
                ppd.makeModel = configMap["printer-make-and-model"]
                ppd.type      = configMap["ppd-type"]
                ppd.file      = configMap["ppd-name"] ?? ""

                // strip out the base file name
                if (ppd.file) {
                     const i = ppd.file.lastIndexOf('/')
                     if (i !== -1) {
                         ppd.pcfile = ppd.file.slice(-(ppd.file.length-i-1))
                     } else {
                         ppd.pcfile = ppd.file
                     }
                } else {
                    ppd.pcfile = ""
                }

                // Not a remote printer and no driver file
                // then immediately offer make/model selection
                if (!configMap["remote"] && ppd.file.length === 0) {
                    openMakeModelDlg()
                }
            }
        }
    }

    Component {
        id: mmComp

        MakeModel {
            anchors.centerIn: parent
            implicitWidth: Math.ceil(parent.width*.85)
            implicitHeight: Math.ceil(parent.height*.85)

            model: ppdModel
            ppdData: Object.assign({}, ppd)

            onSaveValues: ppdMap => {
                Object.assign(ppd, ppdMap)
                driver.text = ppd.type !== PM.PPDType.Manual
                          ? ppd.makeModel
                          : ppd.file

                if (ppd.file.length > 0) {
                    config.set({"ppd-type": ppd.type
                               , "ppd-name": ppd.file})
                    const i = ppd.file.lastIndexOf('/')
                    if (i !== -1) {
                        ppd.pcfile = ppd.file.slice(-(ppd.file.length-i-1))
                    } else {
                        ppd.pcfile = ppd.file
                    }
                } else {
                    config.remove(["ppd-name", "ppd-type"])
                    ppd.pcfile = ""
               }
            }
        }
    }

    component PrinterField: QQC2.TextField {
        Layout.fillWidth: true
        property string orig
        text: orig

        Component.onCompleted: {
            if (addMode) {
                config.add(objectName, text)
            }
        }

        onEditingFinished: {
            if (!addMode) {
                config.remove(objectName)
            }

            if (text !== orig) {
                config.add(objectName, text)
            }
        }
    }

    component PrinterOption: QQC2.CheckBox {
        property bool orig
        checked: orig

        Component.onCompleted: {
            if (addMode) {
                config.add(objectName, checked)
            }
        }

        onToggled: {
            if (!addMode) {
                config.remove(objectName)
            }

            if (checked !== orig) {
                config.add(objectName, checked)
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: modelData.isClass ? "folder-print" : modelData.iconName
                Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
                Layout.preferredHeight: Layout.preferredWidth
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    text: modelData.info ?? ""
                    visible: !addMode
                    level: 3
                    type: Kirigami.Heading.Type.Primary
                }

                Kirigami.Heading {
                    text: modelData.kind.replace("Class", "Group")
                    visible: !addMode
                    level: 5
                    type: Kirigami.Heading.Type.Secondary
                }

                QQC2.Label {
                    text: i18nc("@info:status This is the default printer", "** Default Printer")
                    visible: !addMode && modelData.isDefault
                }

                PrinterOption {
                    objectName: "isDefault"
                    text: i18nc("@action:check Set default printer", "Default printer")
                    orig: modelData.isDefault
                    // CUPS treats default printer independently from other printer attributes
                    // Also, CUPS makes sure it's exclusive, only one can be default and there is
                    // no api for "Not default".

                    // Therefore, if a printer is default, don't allow change for that printer,
                    // only allow it to be set true for a printer that is not default.
                    visible: !modelData.isDefault
                }


                PrinterOption {
                    objectName: "printer-is-shared"
                    text: modelData.isClass
                          ? i18nc("@action:check", "Share this group")
                          : i18nc("@action:check", "Share this printer")
                    enabled: kcm.shareConnectedPrinters
                    orig: modelData.isShared
                }

                PrinterOption {
                    objectName: "printer-is-accepting-jobs"
                    text: i18nc("@action:check", "Accepting print jobs")
                    orig: modelData.isAcceptingJobs
                }
            }
        }

        // Marker (ink) status
        QQC2.ScrollView {
            visible: markersView.count > 0
            Layout.fillWidth: true
            Layout.maximumHeight: Math.floor(root.height/4)
            Layout.preferredHeight: contentHeight + Kirigami.Units.smallSpacing

            Component.onCompleted: {
                if (background) {
                    background.visible = true;
                }
            }

            contentItem: ListView {
                id: markersView
                model: !addMode ? modelData.markers["marker-names"] : null
                clip: true
                delegate: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        text: modelData
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: Kirigami.Units.gridUnit*10
                    }

                    QQC2.ProgressBar {
                        from: 0
                        to: 100
                        value: root.modelData.markers["marker-levels"][index]
                        palette.highlight: root.modelData.markers["marker-colors"][index]
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // Maint actions
        RowLayout {
            visible: !addMode
            Layout.topMargin: Kirigami.Units.largeSpacing

            QQC2.Button {
                text: i18nc("@action:button", "Print Test Page")
                icon.name: "document-print-symbolic"
                onClicked: kcm.printTestPage(modelData.printerName, modelData.isClass)
            }

            QQC2.Button {
                text: i18nc("@action:button", "Print Self-Test Page")
                icon.name: "document-print-symbolic"
                visible: modelData.commands.indexOf("PrintSelfTestPage") !== -1
                onClicked: kcm.printSelfTestPage(modelData.printerName)
            }

            QQC2.Button {
                text: i18nc("@action:button", "Clean Print Heads")
                icon.name: "document-cleanup-symbolic"
                visible: modelData.commands.indexOf("Clean") !== -1
                onClicked: kcm.cleanPrintHeads(modelData.printerName)
            }
        }

        Kirigami.Separator {
            Layout.topMargin: Kirigami.Units.largeSpacing*2
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        GridLayout {
            columns: 2
            columnSpacing: Kirigami.Units.gridUnit

            QQC2.Button {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: i18nc("@action:button", "Find a Printer…")
                icon.name: "search-symbolic"
                visible: addMode && !modelData.isClass

                onClicked: openFindPrinterDlg()
            }

            QQC2.Label {
                text: i18nc("@label:textbox", "Queue Name:")
                Layout.alignment: Qt.AlignRight
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                PrinterField {
                    id: queueName
                    objectName: "printer-name"
                    orig: modelData.printerName
                    enabled: addMode
                    validator: RegularExpressionValidator { regularExpression: /[^/#\\ ]*/ }
                }

                Kirigami.ContextualHelpButton {
                    visible: modelData.isClass
                    toolTipText: xi18nc("@info:whatsthis", "A <interface>printer group</interface> is used to pool printing resources.
                    Member printers can be added to a group and print jobs sent to that group
                    will be dispatched to the appropriate printer.")
                }
            }


            QQC2.Label {
                text: i18nc("@label:textbox", "Description:")
                Layout.alignment: Qt.AlignRight
            }

            PrinterField {
                id: queueInfo
                objectName: "printer-info"
                readOnly: modelData.remote
                orig: modelData.info ?? ""
            }

            QQC2.Label {
                text: i18nc("@label:textbox", "Location:")
                Layout.alignment: Qt.AlignRight
            }

            PrinterField {
                id: location
                objectName: "printer-location"
                readOnly: modelData.remote
                orig: modelData.location ?? ""
            }

            QQC2.Label {
                text: i18nc("@label:textbox", "Connection:")
                Layout.alignment: Qt.AlignRight
                visible: !modelData.isClass
            }

            PrinterField {
                id: devUri
                visible: !modelData.isClass
                objectName: "device-uri"
                orig: modelData.printerUri ?? ""
                readOnly: modelData.remote
            }

            QQC2.Label {
                text: i18nc("@label:listbox", "Member Printers:")
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                visible: modelData.isClass
            }

            // Printer Class member list
            Loader {
                active: modelData.isClass
                visible: active
                Layout.fillHeight: true
                Layout.preferredWidth: Math.round(root.width/2)

                sourceComponent: QQC2.ScrollView {

                    Component.onCompleted: {
                        if (background) {
                            background.visible = true
                        }
                    }

                    contentItem: ListView {
                        id: memberList
                        // cups key for the member list
                        objectName: "member-uris"
                        clip: true

                        property bool showClasses: false

                        model: KItemModels.KSortFilterProxyModel {
                            sourceModel: printerModel

                            filterRowCallback: (source_row, source_parent) => {
                                const ndx = sourceModel.index(source_row, 0, source_parent)
                                const pn = sourceModel.data(ndx, PM.PrinterModel.DestName)

                                if (!memberList.showClasses) {
                                    const isClass = sourceModel.data(ndx, PM.PrinterModel.DestIsClass)
                                    const isRemote = sourceModel.data(ndx, PM.PrinterModel.DestRemote)
                                    if (isClass || isRemote) {
                                        return false
                                    }
                                }
                                return pn !== root.modelData.printerName
                            }
                        }

                        // TODO: Seems to be a timing issue with the delegates and the KSFM.
                        // They're not available right away, so push the setting of
                        // check state a bit later.
                        Component.onCompleted: {
                            if (root.modelData.memberNames.length > 0) {
                                checkTimer.start()
                            }
                        }

                        Timer {
                            id: checkTimer
                            interval: 100; repeat: true; running: false
                            onTriggered: {
                                if (memberList.count > 0) {
                                    stop()
                                    memberList.setChecked()
                                }
                            }
                        }

                        // CUPS strips the queue name from the URI
                        // so for display, compare the queue name.
                        function setChecked() {
                            for (let i=0; i<count; ++i) {
                                const cb = itemAtIndex(i)
                                if (cb instanceof Kirigami.CheckSubtitleDelegate)
                                    cb.checked = cb?.visible
                                        && root.modelData.memberNames.includes(cb.objectName)
                            }
                        }

                        // For save, use the full URI
                        function getChecked(keysOnly: bool) : list<var> {
                            let ret = []
                            for (let i=0; i<count; ++i) {
                                const item = itemAtIndex(i)
                                if (item.checked) {
                                    ret.push(keysOnly ? item.objectName : item.supportedUri)
                                }
                            }
                            return ret
                        }

                        function hasChanges() : bool {
                            let changed = false
                            const vals = getChecked(true)
                            if (vals.length !== root.modelData.memberNames.length
                                    || JSON.stringify(vals) !== JSON.stringify(root.modelData.memberNames)) {
                                changed = true
                            }

                            return changed
                        }

                        delegate: Kirigami.CheckSubtitleDelegate {
                            width: ListView.view.width
                            icon.width: 0
                            objectName: printerName
                            property string supportedUri: uriSupported

                            text: info
                            subtitle: printerName

                            // if there are changes, send checked list
                            // if there are changes and nothing is checked, send empty object
                            // if there are NO changes, just send the checked list
                            onToggled: {
                                const cfg = {}
                                if (memberList.hasChanges()) {
                                    const list = memberList.getChecked()
                                    if (list.length > 0)
                                        cfg[memberList.objectName] = list
                                } else {
                                    cfg[memberList.objectName] = memberList.getChecked()
                                }

                                // an empty member list implies the class should be removed
                                if (Object.keys(cfg).length > 0) {
                                    config.set(cfg)
                                } else {
                                    config.remove(memberList.objectName)
                                }

                            }
                        }
                    }
                }

            }

            QQC2.Label {
                text: i18nc("@label:textbox", "Make/Model:")
                Layout.alignment: Qt.AlignRight
                visible: !modelData.isClass
            }

            RowLayout {
                visible: !modelData.isClass

                QQC2.Label {
                    id: driver
                    text: modelData.kind ?? ""
                }

                QQC2.Button {
                    id: driverSelect
                    text: i18nc("@action:button Select printer make/model", "Select…")
                    icon.name: "printer-symbolic"
                    enabled: !modelData.remote

                    onClicked: {
                        openMakeModelDlg()
                    }
                }
            }

        }

    }

}
