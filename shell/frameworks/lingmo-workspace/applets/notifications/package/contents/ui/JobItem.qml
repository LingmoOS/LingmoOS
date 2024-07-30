/*
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.8
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.notificationmanager as NotificationManager

import org.kde.lingmo.private.notifications 2.0 as Notifications

ColumnLayout {
    id: jobItem

    property int jobState
    property int jobError

    property alias percentage: progressBar.value
    property alias suspendable: suspendButton.visible
    property alias killable: killButton.visible

    property QtObject jobDetails

    readonly property int totalFiles: jobItem.jobDetails && jobItem.jobDetails.totalFiles || 0
    readonly property var url: {
       if (jobItem.jobState !== NotificationManager.Notifications.JobStateStopped
               || jobItem.jobError) {
           return null;
       }

       // For a single file show actions for it
       // Otherwise the destination folder all of them were copied into
       const url = totalFiles === 1 ? jobItem.jobDetails.descriptionUrl
                                    : jobItem.jobDetails.destUrl;

       // Don't offer opening files in Trash
       if (url && url.toString().startsWith("trash:")) {
           return null;
       }

       return url;
   }

    property alias iconContainerItem: jobDragIconItem.parent

    readonly property alias dragging: jobDragArea.dragging
    readonly property alias menuOpen: otherFileActionsMenu.visible

    signal suspendJobClicked
    signal resumeJobClicked
    signal killJobClicked

    signal openUrl(string url)
    signal fileActionInvoked(QtObject action)

    spacing: LingmoUI.Units.smallSpacing

    Notifications.FileInfo {
        id: fileInfo
        url: jobItem.totalFiles === 1 && jobItem.url ? jobItem.url : ""
    }

    // This item is parented to the NotificationItem iconContainer
    Item {
        id: jobDragIconItem
        readonly property bool shown: jobDragIcon.valid
        width: parent ? parent.width : 0
        height: parent ? parent.height : 0
        visible: shown

        Binding {
            target: jobDragIconItem.parent
            property: "visible"
            value: true
            when: jobDragIconItem.shown
            restoreMode: Binding.RestoreBinding
        }

        LingmoUI.Icon {
            id: jobDragIcon

            anchors.fill: parent
            active: jobDragArea.hovered
            opacity: busyIndicator.running ? 0.6 : 1
            source: !fileInfo.error ? fileInfo.iconName : ""

            Behavior on opacity {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            DraggableFileArea {
                id: jobDragArea
                anchors.fill: parent

                dragParent: jobDragIcon
                dragUrl: jobItem.url
                dragPixmap: jobDragIcon.source

                onActivated: jobItem.openUrl(jobItem.url)
                onContextMenuRequested: (pos) => {
                    // avoid menu button glowing if we didn't actually press it
                    otherFileActionsButton.checked = false;

                    otherFileActionsMenu.visualParent = this;
                    otherFileActionsMenu.open(pos.x, pos.y);
                }
            }
        }

        LingmoComponents3.BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            running: fileInfo.busy && !delayBusyTimer.running
            visible: running

            // Avoid briefly flashing the busy indicator
            Timer {
                id: delayBusyTimer
                interval: 500
                repeat: false
                running: fileInfo.busy
            }
        }
    }

    RowLayout {
        id: progressRow
        Layout.fillWidth: true
        // Even when indeterminate, we want to reserve the height for the text, otherwise it's too tightly spaced
        Layout.minimumHeight: progressText.implicitHeight
        // We want largeSpacing between the progress bar and the label
        spacing: LingmoUI.Units.largeSpacing

        LingmoComponents3.ProgressBar {
            id: progressBar
            Layout.fillWidth: true
            from: 0
            to: 100
            // TODO do we actually need the window visible check? perhaps I do because it can be in popup or expanded plasmoid
            indeterminate: visible && Window.window && Window.window.visible && percentage < 1
                           && jobItem.jobState === NotificationManager.Notifications.JobStateRunning
                           // is this too annoying?
                           && (jobItem.jobDetails.processedBytes === 0 || jobItem.jobDetails.totalBytes === 0)
                           && jobItem.jobDetails.processedFiles === 0
                           //&& jobItem.jobDetails.processedDirectories === 0
        }

        LingmoComponents3.Label {
            id: progressText

            visible: !progressBar.indeterminate
            // the || "0" is a workaround for the fact that 0 as number is falsey, and is wrongly considered a missing argument
            // BUG: 451807
            text: i18ndc("lingmo_applet_org.kde.lingmo.notifications", "Percentage of a job", "%1%", jobItem.percentage || "0")
            textFormat: Text.PlainText
        }
    }

    RowLayout {
        id: jobActionsRow
        Layout.fillWidth: true

        spacing: LingmoUI.Units.smallSpacing

        LingmoComponents3.Button {
            id: expandButton

            icon.name: checked ? "collapse-symbolic" : "expand-symbolic"
            text: i18ndc("lingmo_applet_org.kde.lingmo.notifications", "Hides/expands item details", "Details")
            checkable: true
            enabled: jobItem.jobDetails && jobItem.jobDetails.hasDetails
        }

        Item { Layout.fillWidth: true }

        LingmoComponents3.Button {
            id: suspendButton

            icon.name: "media-playback-pause-symbolic"
            text: i18ndc("lingmo_applet_org.kde.lingmo.notifications", "Pause running job", "Pause")
            onClicked: jobItem.jobState === NotificationManager.Notifications.JobStateSuspended ? jobItem.resumeJobClicked()
                                                                                                : jobItem.suspendJobClicked()
        }

        LingmoComponents3.Button {
            id: killButton

            icon.name: "dialog-cancel-symbolic"
            text: i18ndc("lingmo_applet_org.kde.lingmo.notifications", "Cancel running job", "Cancel")
            onClicked: jobItem.killJobClicked()
        }
    }

    Loader {
        Layout.fillWidth: true
        Layout.preferredHeight: item ? item.implicitHeight : 0
        active: expandButton.checked
        // Loader doesn't reset its height when unloaded, just hide it altogether
        visible: active
        sourceComponent: JobDetails {
            jobDetails: jobItem.jobDetails
        }
    }

    Row {
        id: fileActionsRow
        Layout.fillWidth: true
        spacing: LingmoUI.Units.smallSpacing
        // We want the actions to be right-aligned but Row also reverses
        // the order of items, so we put them in reverse order
        layoutDirection: Qt.RightToLeft
        visible: jobItem.url && jobItem.url.toString() !== "" && !fileInfo.error

        LingmoComponents3.Button {
            id: otherFileActionsButton
            height: Math.max(implicitHeight, openButton.implicitHeight)
            icon.name: "application-menu-symbolic"
            checkable: true
            text: openButton.visible ? "" : Accessible.name
            Accessible.name: i18nd("lingmo_applet_org.kde.lingmo.notifications", "More Options…")
            onPressedChanged: {
                if (pressed) {
                    checked = Qt.binding(function() {
                        return otherFileActionsMenu.visible;
                    });
                    otherFileActionsMenu.visualParent = this;
                    // -1 tells it to "align bottom left of visualParent (this)"
                    otherFileActionsMenu.open(-1, -1);
                }
            }

            LingmoComponents3.ToolTip {
                text: parent.Accessible.name
                enabled: parent.text === ""
            }

            Notifications.FileMenu {
                id: otherFileActionsMenu
                url: jobItem.url || ""
                onActionTriggered: jobItem.fileActionInvoked(action)
            }
        }

        LingmoComponents3.Button {
            id: openButton
            width: Math.min(implicitWidth, jobItem.width - otherFileActionsButton.width - fileActionsRow.spacing)
            height: Math.max(implicitHeight, otherFileActionsButton.implicitHeight)
            text: i18nd("lingmo_applet_org.kde.lingmo.notifications", "Open")
            onClicked: jobItem.openUrl(jobItem.url)

            states: [
                State {
                    when: jobItem.jobDetails && jobItem.jobDetails.totalFiles !== 1
                    PropertyChanges {
                        target: openButton
                        text: i18nd("lingmo_applet_org.kde.lingmo.notifications", "Open Containing Folder")
                        icon.name: "folder-open-symbolic"
                    }
                },
                State {
                    when: fileInfo.openAction
                    PropertyChanges {
                        target: openButton
                        text: fileInfo.openAction.text
                        icon.name: fileInfo.openActionIconName
                        visible: fileInfo.openAction.enabled
                        onClicked: {
                            fileInfo.openAction.trigger();
                            jobItem.fileActionInvoked(fileInfo.openAction);
                        }
                    }
                }
            ]
        }
    }

    states: [
        State {
            when: jobItem.jobState === NotificationManager.Notifications.JobStateRunning
            PropertyChanges {
                target: suspendButton
                // Explicitly set it to false so it unchecks when pausing from applet
                // and then the job unpauses programmatically elsewhere.
                checked: false
            }
        },
        State {
            when: jobItem.jobState === NotificationManager.Notifications.JobStateSuspended
            PropertyChanges {
                target: suspendButton
                checked: true
            }
            PropertyChanges {
                target: progressBar
                enabled: false
            }
        },
        State {
            when: jobItem.jobState === NotificationManager.Notifications.JobStateStopped
            PropertyChanges {
                target: progressRow
                visible: false
            }
            PropertyChanges {
                target: jobActionsRow
                visible: false
            }
            PropertyChanges {
                target: expandButton
                checked: false
            }
        }
    ]
}
