/*
 * SPDX-FileCopyrightText: 2021 Devin Lin <devin@kde.org>
 * SPDX-FileCopyrightText: 2018-2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

import org.kde.lingmo.private.mobileshell as MobileShell
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents

import org.kde.notificationmanager as NotificationManager

import org.kde.lingmoui 2.12 as LingmoUI

import org.kde.coreaddons 1.0 as KCoreAddons

// notification properties are in BaseNotificationItem
BaseNotificationItem {
    id: notificationItem
    implicitHeight: mainCard.implicitHeight + mainCard.anchors.topMargin + notificationHeading.height

    // notification heading for groups with one element
    NotificationGroupHeader {
        id: notificationHeading
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        LingmoUI.Theme.colorSet: LingmoUI.Theme.Header
        LingmoUI.Theme.inherit: false

        visible: !notificationItem.inGroup
        height: visible ? implicitHeight : 0

        applicationName: notificationItem.applicationName
        applicationIconSource: notificationItem.applicationIconSource
        originName: notificationItem.originName
    }

    // notification
    NotificationCard {
        id: mainCard
        anchors.topMargin: notificationHeading.visible ? LingmoUI.Units.largeSpacing : 0
        anchors.top: notificationHeading.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        tapEnabled: notificationItem.hasDefaultAction
        onTapped: notificationItem.actionInvoked("default");
        swipeGestureEnabled: notificationItem.closable
        onDismissRequested: notificationItem.close()

        ColumnLayout {
            id: column
            spacing: 0

            // notification summary row
            RowLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: LingmoUI.Units.smallSpacing

                // notification summary
                LingmoComponents.Label {
                    id: summaryLabel
                    Layout.fillWidth: true
                    textFormat: Text.PlainText
                    maximumLineCount: 3
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    text: MobileShell.NotificationsUtils.determineNotificationHeadingText(notificationItem)
                    visible: text !== ""
                    font.weight: Font.DemiBold
                }

                // notification timestamp
                NotificationTimeText {
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                    notificationType: notificationItem.notificationType
                    jobState: notificationItem.jobState
                    jobDetails: notificationItem.jobDetails

                    time: notificationItem.time
                    timeSource: notificationItem.timeSource
                }
            }

            // notification contents
            RowLayout {
                Layout.fillWidth: true
                spacing: LingmoUI.Units.smallSpacing

                // notification text
                NotificationBodyLabel {
                    id: bodyLabel
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    Layout.preferredWidth: column.width - iconContainer.width - LingmoUI.Units.smallSpacing

                    text: notificationItem.body
                }

                // notification icon
                Item {
                    id: iconContainer
                    Layout.preferredWidth: LingmoUI.Units.iconSizes.large
                    Layout.preferredHeight: LingmoUI.Units.iconSizes.large
                    Layout.topMargin: LingmoUI.Units.smallSpacing
                    Layout.bottomMargin: LingmoUI.Units.smallSpacing

                    visible: iconItem.shouldBeShown

                    LingmoUI.Icon {
                        id: iconItem
                        // don't show two identical icons
                        readonly property bool shouldBeShown: valid && source != notificationItem.applicationIconSource
                        anchors.fill: parent
                        smooth: true
                        // don't show a generic "info" icon since this is a notification already
                        source: notificationItem.icon !== "dialog-information" ? notificationItem.icon : ""
                        visible: shouldBeShown
                    }
                }
            }

            // Job progress reporting
            Loader {
                id: jobLoader
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight : 0
                active: notificationItem.notificationType === NotificationManager.Notifications.JobType
                visible: active
                sourceComponent: NotificationJobItem {
                    iconContainerItem: iconContainer

                    jobState: notificationItem.jobState
                    jobError: notificationItem.jobError
                    percentage: notificationItem.percentage
                    suspendable: notificationItem.suspendable
                    killable: notificationItem.killable

                    jobDetails: notificationItem.jobDetails

                    onSuspendJobClicked: notificationItem.suspendJobClicked()
                    onResumeJobClicked: notificationItem.resumeJobClicked()
                    onKillJobClicked: notificationItem.killJobClicked()

                    onOpenUrl: notificationItem.openUrl(url)
                    onFileActionInvoked: notificationItem.fileActionInvoked(action)
                }
            }

            // notification actions
            NotificationFooterActions {
                Layout.fillWidth: true
                Layout.topMargin: LingmoUI.Units.smallSpacing
                notification: notificationItem
            }

            // thumbnails
            Loader {
                id: thumbnailStripLoader
                Layout.topMargin: LingmoUI.Units.largeSpacing
                Layout.fillWidth: true
                active: notificationItem.urls.length > 0
                visible: active
                asynchronous: true
                sourceComponent: ThumbnailStrip {
                    leftPadding: -thumbnailStripLoader.Layout.leftMargin
                    rightPadding: -thumbnailStripLoader.Layout.rightMargin
                    topPadding: -notificationItem.thumbnailTopPadding
                    bottomPadding: -thumbnailStripLoader.Layout.bottomMargin
                    urls: notificationItem.urls
                    onOpenUrl: notificationItem.openUrl(url)
                    onFileActionInvoked: notificationItem.fileActionInvoked(action)
                }
            }
        }
    }
}
