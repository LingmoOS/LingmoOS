/*
 *    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *    SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.private.keyboardindicator as KeyboardIndicator
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.extras as LingmoExtras

PlasmoidItem {
    id: root

    readonly property KeyboardIndicator.KeyState capsLockState: Plasmoid.configuration.key.includes("Caps Lock") ? kiComponent.createObject(null, {key: Qt.Key_CapsLock}) : null
    readonly property KeyboardIndicator.KeyState numLockState: Plasmoid.configuration.key.includes("Num Lock") ? kiComponent.createObject(null, {key: Qt.Key_NumLock}) : null

    Plasmoid.icon: {
        if (capsLockState?.locked && numLockState?.locked) {
            return "input-combo-on";
        } else if (capsLockState?.locked) {
            return "input-caps-on";
        } else if (numLockState?.locked) {
            return "input-num-on";
        } else {
            return "input-caps-on";
        }
    }

    Component {
        id: kiComponent
        KeyboardIndicator.KeyState { }
    }

    // Only exists because the default CompactRepresentation doesn't expose a
    // way to mark its icon as disabled.
    // TODO remove once it gains that feature.
    compactRepresentation: MouseArea {
        id: compactMouse

        activeFocusOnTab: true
        hoverEnabled: true

        Accessible.name: Plasmoid.title
        Accessible.description: root.toolTipSubText
        Accessible.role: Accessible.Button

        property bool wasExpanded: false
        onPressed: wasExpanded = root.expanded
        onClicked: root.expanded = !wasExpanded

        LingmoUI.Icon {
            anchors.fill: parent
            source: Plasmoid.icon
            active: compactMouse.containsMouse
            enabled: root.capsLockState?.locked || root.numLockState?.locked || false
        }
    }

    fullRepresentation: LingmoComponents.Page {
        implicitWidth: LingmoUI.Units.gridUnit * 12
        implicitHeight: LingmoUI.Units.gridUnit * 12

        LingmoExtras.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (LingmoUI.Units.gridUnit * 4)
            iconName: Plasmoid.icon
            text: root.toolTipSubText
        }
    }

    switchWidth: LingmoUI.Units.gridUnit * 12
    switchHeight: LingmoUI.Units.gridUnit * 12

    Plasmoid.status: root.capsLockState?.locked || root.numLockState?.locked
        ? LingmoCore.Types.ActiveStatus
        : LingmoCore.Types.HiddenStatus

    toolTipSubText: {
        let text = [];
        if (root.capsLockState?.locked) {
            text.push(i18n("Caps Lock activated"));
        }
        if (root.numLockState?.locked) {
            text.push(i18n("Num Lock activated"));
        }
        if (text.length > 0) {
            // Not using KUIT markup for these newline characters because those
            // get translated into HTML, and this text is displayed in the applet's
            // tooltip which does not render HTML at all for security reasons
            return text.join("\n");
        } else {
            return i18n("No lock keys activated");
        }
    }
}
