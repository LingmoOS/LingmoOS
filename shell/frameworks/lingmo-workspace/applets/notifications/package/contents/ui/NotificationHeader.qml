/*
    SPDX-FileCopyrightText: 2018-2019 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.8
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.notificationmanager as NotificationManager

import org.kde.coreaddons 1.0 as KCoreAddons

import org.kde.quickcharts 1.0 as Charts

import "global"

RowLayout {
    id: notificationHeading
    property bool inGroup
    property bool inHistory
    property int notificationType

    property var applicationIconSource
    property string applicationName
    property string originName

    property string configureActionLabel

    property alias configurable: configureButton.visible
    property alias dismissable: dismissButton.visible
    property bool dismissed
    property alias closeButtonTooltip: closeButtonToolTip.text
    property alias closable: closeButton.visible

    property var time

    property int jobState
    property QtObject jobDetails

    property real timeout: 5000
    property real remainingTime: 0

    signal configureClicked
    signal dismissClicked
    signal closeClicked

    // notification created/updated time changed
    onTimeChanged: updateAgoText()

    function updateAgoText() {
        ageLabel.agoText = ageLabel.generateAgoText();
    }

    spacing: LingmoUI.Units.smallSpacing
    Layout.preferredHeight: Math.max(applicationNameLabel.implicitHeight, LingmoUI.Units.iconSizes.small)

    Component.onCompleted: updateAgoText()

    Connections {
        target: Globals
        // clock time changed
        function onTimeChanged() {
            notificationHeading.updateAgoText()
        }
    }

    LingmoUI.Icon {
        id: applicationIconItem
        Layout.preferredWidth: LingmoUI.Units.iconSizes.small
        Layout.preferredHeight: LingmoUI.Units.iconSizes.small
        source: notificationHeading.applicationIconSource
        visible: valid
    }

    LingmoUI.Heading {
        id: applicationNameLabel
        Layout.fillWidth: true
        level: 5
        opacity: 0.9
        textFormat: Text.PlainText
        elide: Text.ElideMiddle
        maximumLineCount: 2
        text: notificationHeading.applicationName + (notificationHeading.originName ? " · " + notificationHeading.originName : "")
    }

    Item {
        id: spacer
        Layout.fillWidth: true
    }

    LingmoUI.Heading {
        id: ageLabel

        // the "n minutes ago" text, for jobs we show remaining time instead
        // updated periodically by a Timer hence this property with generate() function
        property string agoText: ""
        visible: text !== ""
        level: 5
        opacity: 0.9
        wrapMode: Text.NoWrap
        text: generateRemainingText() || agoText
        textFormat: Text.PlainText

        function generateAgoText() {
            if (!time || isNaN(time.getTime())
                    || notificationHeading.jobState === NotificationManager.Notifications.JobStateRunning
                    || notificationHeading.jobState === NotificationManager.Notifications.JobStateSuspended) {
                return "";
            }

            var deltaMinutes = Math.floor((Date.now() - time.getTime()) / 1000 / 60);
            if (deltaMinutes < 1) {
                // "Just now" is implied by
                return notificationHeading.inHistory
                    ? i18ndc("lingmo_applet_org.kde.lingmo.notifications", "Notification was added less than a minute ago, keep short", "Just now")
                    : "";
            }

            // Received less than an hour ago, show relative minutes
            if (deltaMinutes < 60) {
                return i18ndcp("lingmo_applet_org.kde.lingmo.notifications", "Notification was added minutes ago, keep short", "%1 min ago", "%1 min ago", deltaMinutes);
            }
            // Received less than a day ago, show time, 22 hours so the time isn't as ambiguous between today and yesterday
            if (deltaMinutes < 60 * 22) {
                return Qt.formatTime(time, Qt.locale().timeFormat(Locale.ShortFormat).replace(/.ss?/i, ""));
            }

            // Otherwise show relative date (Yesterday, "Last Sunday", or just date if too far in the past)
            return KCoreAddons.Format.formatRelativeDate(time, Locale.ShortFormat);
        }

        function generateRemainingText() {
            if (notificationHeading.notificationType !== NotificationManager.Notifications.JobType
                || notificationHeading.jobState !== NotificationManager.Notifications.JobStateRunning) {
                return "";
            }

            var details = notificationHeading.jobDetails;
            if (!details || !details.speed) {
                return "";
            }

            var remaining = details.totalBytes - details.processedBytes;
            if (remaining <= 0) {
                return "";
            }

            var eta = remaining / details.speed;
            if (eta < 0.5) { // Avoid showing "0 seconds remaining"
                return "";
            }

            if (eta < 60) { // 1 minute
                return i18ndcp("lingmo_applet_org.kde.lingmo.notifications", "seconds remaining, keep short",
                              "%1 s remaining", "%1 s remaining", Math.round(eta));
            }
            if (eta < 60 * 60) {// 1 hour
                return i18ndcp("lingmo_applet_org.kde.lingmo.notifications", "minutes remaining, keep short",
                              "%1 min remaining", "%1 min remaining",
                              Math.round(eta / 60));
            }
            if (eta < 60 * 60 * 5) { // 5 hours max, if it takes even longer there's no real point in showing that
                return i18ndcp("lingmo_applet_org.kde.lingmo.notifications", "hours remaining, keep short",
                              "%1 h remaining", "%1 h remaining",
                              Math.round(eta / 60 / 60));
            }

            return "";
        }

        LingmoCore.ToolTipArea {
            anchors.fill: parent
            active: ageLabel.agoText !== ""
            subText: notificationHeading.time ? notificationHeading.time.toLocaleString(Qt.locale(), Locale.LongFormat) : ""
        }
    }

    LingmoComponents3.ToolButton {
        id: configureButton
        icon.name: "configure"
        visible: false

        display: LingmoComponents3.AbstractButton.IconOnly
        text: notificationHeading.configureActionLabel || i18nd("lingmo_applet_org.kde.lingmo.notifications", "Configure")
        Accessible.description: applicationNameLabel.text

        onClicked: notificationHeading.configureClicked()

        LingmoComponents3.ToolTip {
            text: parent.text
        }
    }

    LingmoComponents3.ToolButton {
        id: dismissButton
        icon.name: notificationHeading.dismissed ? "window-restore" : "window-minimize"
        visible: false

        display: LingmoComponents3.AbstractButton.IconOnly
        text: notificationHeading.dismissed
            ? i18ndc("lingmo_applet_org.kde.lingmo.notifications", "Opposite of minimize", "Restore")
            : i18nd("lingmo_applet_org.kde.lingmo.notifications", "Minimize")
        Accessible.description: applicationNameLabel.text

        onClicked: notificationHeading.dismissClicked()

        LingmoComponents3.ToolTip {
            text: parent.text
        }
    }

    LingmoComponents3.ToolButton {
        id: closeButton
        visible: false
        icon.name: "window-close"

        display: LingmoComponents3.AbstractButton.IconOnly
        text: closeButtonToolTip.text
        Accessible.description: applicationNameLabel.text

        onClicked: notificationHeading.closeClicked()

        LingmoComponents3.ToolTip {
            id: closeButtonToolTip
            text: i18nd("lingmo_applet_org.kde.lingmo.notifications", "Close")
        }

        Charts.PieChart {
            id: chart
            anchors.fill: parent.contentItem
            anchors.margins: 1

            opacity: (notificationHeading.remainingTime > 0 && notificationHeading.remainingTime < notificationHeading.timeout) ? 1 : 0
            Behavior on opacity {
                NumberAnimation { duration: LingmoUI.Units.longDuration }
            }

            range { from: 0; to: notificationHeading.timeout; automatic: false }

            valueSources: Charts.SingleValueSource { value: notificationHeading.remainingTime }
            colorSource: Charts.SingleValueSource { value: LingmoUI.Theme.highlightColor }

            thickness: 5

            transform: Scale { origin.x: chart.width / 2; xScale: -1 }
        }
    }

    states: [
        State {
            when: notificationHeading.inGroup
            PropertyChanges {
                target: applicationIconItem
                source: ""
            }
            PropertyChanges {
                target: applicationNameLabel
                visible: false
            }
        }

    ]
}
