/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kitemmodels 1.0

import org.kde.kcmutils as KCM
import org.kde.private.kcm.kscreen 1.0 as KScreen

KCM.SimpleKCM {
    id: root

    property int selectedOutput: 0
    property int revertCountdown: 15

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 30

    actions: Kirigami.Action {
        text: i18nc("@action:button Briefly show the display name in a popup label on each screen", "Identify Screens")
        icon.name: "documentinfo-symbolic"
        visible: kcm.multipleScreensAvailable
        onTriggered: kcm.identifyOutputs()
    }

    topPadding: 0
    leftPadding: 0
    rightPadding: 0

    // This is to fix Output dragging
    flickable.interactive: Kirigami.Settings.hasTransientTouchInput

    Kirigami.PromptDialog {
        id: confirmMsg
        title: i18n("Keep display configuration?")
        onVisibleChanged: {
            if (visible) {
                revertButton.forceActiveFocus()
            } else {
                revertTimer.stop();
            }
        }
        showCloseButton: false
        subtitle: i18np("Will revert to previous configuration in %1 second.",
                        "Will revert to previous configuration in %1 seconds.",
                        revertCountdown);

        parent: root.parent?.contentItem ?? root.parent

        footer: QQC2.DialogButtonBox {
            id: confirmDialogButtonBox
            QQC2.Button {
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                        event.accepted = true;
                        clicked();
                    }
                }
                icon.name: "dialog-ok"
                text: i18n("&Keep")
            }
            QQC2.Button {
                id: revertButton
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                        event.accepted = true;
                        clicked();
                    }
                }
                action: QQC2.Action {
                    icon.name: "edit-undo"
                    text: i18n("&Revert")
                    shortcut: "Escape"
                    enabled: confirmMsg.visible
                }
            }
            onAccepted: {
                confirmMsg.close();
            }
            onRejected: {
                revertTimer.stop();
                kcm.setStopUpdatesFromBackend(false);
                kcm.revertSettings();
            }
        }
    }

    Connections {
        target: kcm
        function onInvalidConfig(reason) {
            if (reason === KScreen.KCMKScreen.NoEnabledOutputs) {
                invalidConfigMsg.text = i18nc("@info", "All displays are disabled. Enable at least one.")
            } else if (reason === KScreen.KCMKScreen.ConfigHasGaps) {
                invalidConfigMsg.text = i18nc("@info", "Gaps between displays are not supported. Make sure all displays are touching.")
            }
            invalidConfigMsg.visible = true;
        }
        function onErrorOnSave() {
            errSaveMsg.visible = true;
        }
        function onGlobalScaleWritten() {
            scaleMsg.visible = true;
        }
        function onOutputConnect(connected) {
            if (connected) {
                connectMsg.text = i18n("A new output has been added. Settings have been reloaded.");
            } else {
                connectMsg.text = i18n("An output has been removed. Settings have been reloaded.");
            }
            connectMsg.visible = true;
        }
        function onBackendError() {
            errBackendMsg.visible = true;
        }
        function onSettingsReverted() {
            confirmMsg.close();
            revertMsg.visible = true;
        }
        function onShowRevertWarning() {
            revertCountdown = 15;
            confirmMsg.open();
            revertTimer.restart();
        }
        function onChanged() {
            invalidConfigMsg.visible = false;
            errSaveMsg.visible = false;
            scaleMsg.visible = false;
            revertMsg.visible = false;
        }
    }

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: ColumnLayout {
        spacing: 0

        Kirigami.InlineMessage {
            id: invalidConfigMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Error
            showCloseButton: true
        }
        Kirigami.InlineMessage {
            id: errBackendMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Error
            text: i18n("No KScreen backend found. Please check your KScreen installation.")
            visible: false
            showCloseButton: false
        }
        Kirigami.InlineMessage {
            id: errSaveMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Error
            text: i18n("Outputs could not be saved due to error.")
            visible: false
            showCloseButton: true
        }
        Kirigami.InlineMessage {
            id: scaleMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Information
            text: i18n("Global scale changes will come into effect after the system is restarted.")
            visible: false
            showCloseButton: true
            actions: [
                Kirigami.Action {
                    icon.name: "system-reboot"
                    text: i18n("Restart")
                    onTriggered: kcm.requestReboot();
                }
            ]
        }
        Kirigami.InlineMessage {
            id: connectMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Information
            visible: false
            showCloseButton: true
        }
        Kirigami.InlineMessage {
            id: revertMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Information
            text: i18n("Display configuration reverted.")
            visible: false
            showCloseButton: true
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Dialog {
            id: reorderDialog

            title: i18nc("@title:window", "Change Priorities")
            showCloseButton: true
            standardButtons: Kirigami.Dialog.Ok

            contentItem: ListView {
                id: reorderView

                implicitWidth: Math.min(root.width * 0.75, Kirigami.Units.gridUnit * 32)
                implicitHeight: contentHeight

                reuseItems: true
                model: KSortFilterProxyModel {
                    id: enabledOutputsModel
                    sourceModel: kcm.outputModel
                    filterRoleName: "enabled"
                    filterString: "true"
                    sortRoleName: "priority"
                    sortOrder: Qt.AscendingOrder
                }
                delegate: Kirigami.SwipeListItem {
                    id: delegate

                    property var output: model

                    width: ListView.view.width

                    background: null
                    contentItem: KD.TitleSubtitle {
                        title: delegate.output.display
                        subtitle: (delegate.output.priority === 1) ? i18n("Primary") : ""
                    }
                    actions: [
                        Kirigami.Action {
                            icon.name: "arrow-up"
                            text: i18n("Raise priority")
                            enabled: delegate.output.priority > 1
                            onTriggered: {
                                if (enabled) {
                                    delegate.output.priority -= 1;
                                }
                            }
                        },
                        Kirigami.Action {
                            icon.name: "arrow-down"
                            text: i18n("Lower priority")
                            enabled: delegate.output.priority < reorderView.count
                            onTriggered: {
                                if (enabled) {
                                    delegate.output.priority += 1;
                                }
                            }
                        }
                    ]
                }
            }
        }

        Connections {
            target: kcm
            function onInvalidConfig(reason) {
                if (reason === KScreen.KCMKScreen.NoEnabledOutputs) {
                    invalidConfigMsg.text = i18nc("@info", "All displays are disabled. Enable at least one.")
                } else if (reason === KScreen.KCMKScreen.ConfigHasGaps) {
                    invalidConfigMsg.text = i18nc("@info", "Gaps between displays are not supported. Make sure all displays are touching.")
                }
                invalidConfigMsg.visible = true;
            }
            function onErrorOnSave() {
                errSaveMsg.visible = true;
            }
            function onGlobalScaleWritten() {
                scaleMsg.visible = true;
            }
            function onOutputConnect(connected) {
                root.selectedOutput = 0;
                if (connected) {
                    connectMsg.text = i18n("A new output has been added. Settings have been reloaded.");
                } else {
                    connectMsg.text = i18n("An output has been removed. Settings have been reloaded.");
                }
                connectMsg.visible = true;
            }
            function onBackendError() {
                errBackendMsg.visible = true;
            }
            function onSettingsReverted() {
                confirmMsg.close();
                revertMsg.visible = true;
            }
            function onShowRevertWarning() {
                revertCountdown = 15;
                confirmMsg.open();
                revertTimer.restart();
            }
            function onChanged() {
                invalidConfigMsg.visible = false;
                errSaveMsg.visible = false;
                scaleMsg.visible = false;
                revertMsg.visible = false;
            }
        }

        Rectangle {
            Layout.preferredHeight: Math.max(root.height * 0.35, Kirigami.Units.gridUnit * 12)
            Layout.fillWidth: true
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            color: Kirigami.Theme.backgroundColor

            ScreenView {
                id: screen

                anchors.fill: parent
                enabled: kcm.outputModel && kcm.backendReady
                outputs: kcm.outputModel
            }

            Kirigami.Separator {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
            }
        }

        Panel {
            enabled: kcm.outputModel && kcm.backendReady
            Layout.fillWidth: true
            enabledOutputs: enabledOutputsModel
            selectedOutput: root.selectedOutput
            onSelectedOutputChanged: {
                root.selectedOutput = selectedOutput;
                selectedOutput = Qt.binding(() => root.selectedOutput);
            }
            onReorder: reorderDialog.open()
        }

        Timer {
            id: revertTimer
            interval: 1000
            running: false
            repeat: true

            onTriggered: {
                revertCountdown -= 1;
                if (revertCountdown < 1) {
                    confirmDialogButtonBox.rejected();
                }
            }
        }
    }
}
