/*
 *   SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.15
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.lingmo5support 2.0 as P5Support
import org.kde.kquickcontrolsaddons 2.0 as QtExtra
import org.kde.lingmo.private.timer 0.1 as TimerPlasmoid
import org.kde.notification 1.0

PlasmoidItem {
    id: root;

    switchWidth: LingmoUI.Units.gridUnit * 8
    switchHeight: LingmoUI.Units.gridUnit * 4

    readonly property bool inPanel: [LingmoCore.Types.TopEdge, LingmoCore.Types.RightEdge, LingmoCore.Types.BottomEdge, LingmoCore.Types.LeftEdge]
        .includes(Plasmoid.location)
    readonly property bool isVertical: Plasmoid.formFactor === LingmoCore.Types.Vertical

    readonly property variant predefinedTimers: plasmoid.configuration.predefinedTimers;

    Plasmoid.backgroundHints: LingmoCore.Types.ShadowBackground | LingmoCore.Types.ConfigurableBackground

    // Display remaining time (hours and minutes) (default: enabled)
    readonly property bool showRemainingTime: Plasmoid.configuration.showRemainingTime

    // Display seconds in addition to hours and minutes (default: enabled)
    readonly property bool showSeconds: Plasmoid.configuration.showSeconds
    property int seconds : restoreToSeconds(plasmoid.configuration.running, plasmoid.configuration.savedAt, plasmoid.configuration.seconds);

    // Display timer toggle control (default: enabled)
    readonly property bool showTimerToggle: Plasmoid.configuration.showTimerToggle

    // Display progress bar (default: disabled)
    readonly property bool showProgressBar: Plasmoid.configuration.showProgressBar

    // show notification on timer completion (default: enabled)
    property bool showNotification: plasmoid.configuration.showNotification;
    // run custom command on timer completion (default: disabled)
    property bool runCommand: plasmoid.configuration.runCommand;
    property string command: plasmoid.configuration.command;

    // show title (can be customized in the settings dialog, default: disabled)
    readonly property bool showTitle: plasmoid.configuration.showTitle;
    readonly property string title: plasmoid.configuration.title;
    readonly property bool alertMode: root.running && root.seconds < 60
    property bool running: plasmoid.configuration.running > 0
    property bool suspended: false;

    readonly property string notificationText: plasmoid.configuration.notificationText;

    toolTipMainText: {
        var timerName = "";
        if (showTitle && title != "") {
            timerName = title;
        } else {
            timerName = Plasmoid.title;
        }

        if (running) {
            return i18n("%1 is running", timerName);
        } else {
            return i18n("%1 not running", timerName);
        }
    }
    toolTipSubText: running ? i18np("Remaining time left: %1 second", "Remaining time left: %1 seconds", seconds) : i18n("Use mouse wheel to change digits or choose from predefined timers in the context menu");

    compactRepresentation: CompactRepresentation { }
    fullRepresentation: TimerView { }

    function toggleTimer() {
        if (root.running) {
            root.stopTimer();
        } else {
            root.startTimer();
        }
    }

    Notification {
        id: timerNotification
        componentName: "lingmo_applet_timer"
        eventId: "timerFinished"
        title: root.title || i18n("Timer")
        text: notificationText || i18n("Timer finished")
    }

    Timer {
        id: t;
        interval: 1000;
        onTriggered: {
            if (root.seconds != 0) {
                root.seconds--;
            }
            if (root.seconds == 0) {
                root.running = false;

                if (showNotification) {
                    timerNotification.sendEvent();
                }
                if (runCommand) {
                    TimerPlasmoid.Timer.runCommand(command);
                }
                saveTimer();
            }
        }
        repeat: true;
        running: root.running;
    }

    Timer {
        id: delayedSaveTimer;
        interval: 3000;
        onTriggered: saveTimer();
    }

    function onDigitHasChanged() {
        delayedSaveTimer.stop();
        delayedSaveTimer.start();
    }

    Plasmoid.contextualActions: [
        LingmoCore.Action {
            id: startAction
            text: i18nc("@action", "&Start")
            onTriggered: startTimer()
        },
        LingmoCore.Action {
            id: stopAction
            text: i18nc("@action", "S&top")
            onTriggered: stopTimer()
        },
        LingmoCore.Action {
            id: resetAction
            text: i18nc("@action", "&Reset")
            onTriggered: resetTimer()
        },
        LingmoCore.Action {
            id: separator1
            isSeparator: true
        },
        LingmoCore.Action {
            id: separator2
            isSeparator: true
        }
    ]

    Instantiator {
        model: plasmoid.configuration.predefinedTimers
        delegate: LingmoCore.Action {
            text: TimerPlasmoid.Timer.secondsToString(modelData, "hh:mm:ss")
            onTriggered: {
                seconds = modelData
                startTimer();
            }
        }
        onObjectAdded: (index, object) => {
            Plasmoid.contextualActions.splice(Plasmoid.contextualActions.indexOf(separator2), 0, object)
        }
        onObjectRemoved: (index, object) => {
            Plasmoid.contextualActions.splice(Plasmoid.contextualActions.indexOf(object), 1)
        }
    }

    function startTimer() {
        running = true;
        suspended = false;
        opacityNeedsReset();
        saveTimer();
    }

    function stopTimer() {
        running = false;
        suspended = true;
        saveTimer();
    }

    function resetTimer() {
        running = false;
        suspended = false;
        seconds = 0;
        opacityNeedsReset();
        saveTimer();
    }

    signal opacityNeedsReset()
    signal digitHasChanged()

    function saveTimer() {
        plasmoid.configuration.running = running ? seconds : 0;
        plasmoid.configuration.savedAt = new Date();
        plasmoid.configuration.seconds = seconds
    }

    function restoreToSeconds(cRunning, cSavedAt, cSeconds) {
        if (cRunning > 0) {
            var elapsedSeconds = cRunning - ~~(~~(((new Date()).getTime() - cSavedAt.getTime()) / 1000));
            if (elapsedSeconds >= 0) {
                return elapsedSeconds;
            } else {
                return 0;
            }
        } else {
            return cSeconds;
        }
    }
}

