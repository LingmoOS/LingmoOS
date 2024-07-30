/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as QQC2 // For StackView
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmo.lingmo5support 2.0 as P5Support
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.lingmo.private.clipboard 0.1 as Private

PlasmoidItem {
    id: main

    readonly property bool isClipboardEmpty: historyModel.count === 0
    property bool editing: false
    property alias clearHistoryAction: clearAction

    signal clearSearchField

    switchWidth: LingmoUI.Units.gridUnit * 5
    switchHeight: LingmoUI.Units.gridUnit * 5
    Plasmoid.status: isClipboardEmpty ? LingmoCore.Types.PassiveStatus : LingmoCore.Types.ActiveStatus
    toolTipMainText: i18n("Clipboard Contents")
    toolTipSubText: isClipboardEmpty ? i18n("Clipboard is empty") : clipboardSource.data["clipboard"]["current"]
    toolTipTextFormat: Text.PlainText
    Plasmoid.icon: "klipper-symbolic"

    function action_configure() {
        clipboardSource.service("", "configureKlipper");
    }

    function action_clearHistory() {
        historyModel.clearHistory();
        clearSearchField()
    }

    onIsClipboardEmptyChanged: {
        if (isClipboardEmpty) {
            // We need to hide the applet before changing its status to passive
            // because only the active applet can hide itself
            if (main.hideOnWindowDeactivate)
                main.expanded = false;
            Plasmoid.status = LingmoCore.Types.HiddenStatus;
        } else {
            Plasmoid.status = LingmoCore.Types.ActiveStatus
        }
    }

    Plasmoid.contextualActions: [
        LingmoCore.Action {
            id: clearAction
            text: i18n("Clear History")
            icon.name: "edit-clear-history"
            visible: !main.isClipboardEmpty && !main.editing
            onTriggered: {
                historyModel.clearHistory();
                clearSearchField()
            }
        }
    ]

    LingmoCore.Action {
        id: configureAction
        text: i18n("Configure Clipboard…")
        icon.name: "configure"
        shortcut: "alt+d, s"
        onTriggered: {
            clipboardSource.service("", "configureKlipper")
        }
    }

    Component.onCompleted: {
        Plasmoid.setInternalAction("configure", configureAction);
    }

    P5Support.DataSource {
        id: clipboardSource
        property bool editing: false;
        engine: "clipboard"
        connectedSources: "clipboard"
        function service(uuid, op) {
            var service = clipboardSource.serviceForSource(uuid);
            var operation = service.operationDescription(op);
            return service.startOperationCall(operation);
        }
        function edit(uuid, text) {
            clipboardSource.editing = true;
            const service = clipboardSource.serviceForSource(uuid);
            const operation = service.operationDescription("edit");
            operation.text = text;
            const job = service.startOperationCall(operation);
            job.finished.connect(function() {
                clipboardSource.editing = false;
            });
        }
    }

    Private.HistoryModel {
        id: historyModel
    }

    fullRepresentation: LingmoExtras.Representation {
        id: dialogItem
        Layout.minimumWidth: LingmoUI.Units.gridUnit * 24
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 24
        Layout.maximumWidth: LingmoUI.Units.gridUnit * 80
        Layout.maximumHeight: LingmoUI.Units.gridUnit * 40
        collapseMarginsHint: true

        focus: true

        header: stack.currentItem.header

        property alias listMargins: listItemSvg.margins
        readonly property var appletInterface: main

        KSvg.FrameSvgItem {
            id : listItemSvg
            imagePath: "widgets/listitem"
            prefix: "normal"
            visible: false
        }

        Keys.forwardTo: [stack.currentItem]

        QQC2.StackView {
            id: stack
            anchors.fill: parent
            initialItem: ClipboardPage {
            }
        }
    }
}
