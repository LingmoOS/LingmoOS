/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2021 Devin Lin <devin@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.lingmoui 2.12 as LingmoUI

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.private.nanoshell 2.0 as NanoShell
import org.kde.lingmo.private.mobileshell as MobileShell
import org.kde.lingmo.private.mobileshell.state as MobileShellState
import org.kde.lingmo.components 3.0 as LingmoComponents

MobileShell.BaseItem {
    id: root

    required property bool restrictedPermissions

    // Model interface
    required property string text
    required property string status
    required property string icon
    required property bool enabled
    required property string settingsCommand
    required property var toggleFunction

    signal closeRequested()

    // set by children
    property var iconItem

    readonly property color enabledButtonBorderColor: Qt.darker(LingmoUI.Theme.highlightColor, 1.25)
    readonly property color disabledButtonBorderColor: LingmoUI.ColorUtils.tintWithAlpha(LingmoUI.Theme.textColor, LingmoUI.Theme.backgroundColor, 0.75)
    readonly property color enabledButtonColor: LingmoUI.ColorUtils.tintWithAlpha(LingmoUI.Theme.highlightColor, LingmoUI.Theme.backgroundColor, 0.6)
    readonly property color enabledButtonPressedColor: LingmoUI.ColorUtils.tintWithAlpha(LingmoUI.Theme.highlightColor, LingmoUI.Theme.backgroundColor, 0.4);
    readonly property color disabledButtonColor: LingmoUI.Theme.backgroundColor
    readonly property color disabledButtonPressedColor: Qt.darker(disabledButtonColor, 1.1)

    // scale animation on press
    property real zoomScale: 1
    Behavior on zoomScale {
        NumberAnimation {
            duration: 200
            easing.type: Easing.OutExpo
        }
    }

    transform: Scale {
        origin.x: root.width / 2;
        origin.y: root.height / 2;
        xScale: root.zoomScale
        yScale: root.zoomScale
    }

    function delegateClick() {
        if (root.toggle) {
            root.toggle();
        } else if (root.toggleFunction) {
            root.toggleFunction();
        } else if (root.settingsCommand && !root.restrictedPermissions) {
            closeRequested();

            MobileShellState.ShellDBusClient.openAppLaunchAnimationWithPosition(
                __getCurrentScreenNumber(),
                root.icon,
                root.text,
                'org.kde.mobile.lingmosettings', // settings window id
                -1,
                -1,
                Math.min(root.iconItem.width, root.iconItem.height));
            MobileShell.ShellUtil.executeCommand(root.settingsCommand);
        }
    }

    function delegatePressAndHold() {
        if (root.settingsCommand && !root.restrictedPermissions) {
            closeRequested();
            MobileShellState.ShellDBusClient.openAppLaunchAnimationWithPosition(
                __getCurrentScreenNumber(),
                root.icon,
                root.text,
                'org.kde.mobile.lingmosettings', // settings window id
                -1,
                -1,
                Math.min(root.iconItem.width, root.iconItem.height));
            MobileShell.ShellUtil.executeCommand(root.settingsCommand);
        } else if (root.toggleFunction) {
            root.toggleFunction();
        }
    }

    function __getCurrentScreenNumber() {
        const screens = Qt.application.screens;
        for (let i = 0; i < screens.length; i++) {
            if (screens[i].name === Screen.name) {
                return i;
            }
        }

        return 0;
    }
}
