/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kwin as KWinComponents

KWinComponents.SceneEffect {
    id: effect

    property bool activated: false
    readonly property int animationDuration: Kirigami.Units.longDuration

    // stored as a property to keep the component alive and cleanup afterwards
    property var mainDelegate: KWinComponents.Workspace.desktops.length < 3 ? Qt.createComponent("PlaceholderView.qml") : Qt.createComponent("ScreenView.qml")
    delegate: mainDelegate
    Instantiator {
        model: effect.configuration.BorderActivate
        KWinComponents.ScreenEdgeHandler {
            mode: KWinComponents.ScreenEdgeHandler.Pointer
            edge: modelData
            onActivated: toggle();
        }
    }

    Instantiator {
        model: effect.configuration.TouchBorderActivate
        KWinComponents.ScreenEdgeHandler {
            mode: KWinComponents.ScreenEdgeHandler.Touch
            edge: modelData
            onActivated: toggle();
        }
    }

    KWinComponents.ShortcutHandler {
        name: "Cube"
        text: i18nd("kwin_effect_cube", "Toggle Cube")
        sequence: "Meta+C"
        onActivated: toggle();
    }

    Timer {
        id: deactivateTimer
        interval: effect.animationDuration
        onTriggered: effect.visible = false
    }

    function toggle() {
        if (activated) {
            deactivate();
        } else {
            activate();
        }
    }

    function activate() {
        if (activated || deactivateTimer.running) {
            return;
        }
        visible = true;
        activated = true;
    }

    function deactivate() {
        if (!activated) {
            return;
        }
        activated = false;
        deactivateTimer.start();
    }
}
