/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQml
import org.kde.qqc2desktopstyle.private as StylePrivate
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.ScrollBar {
    id: controlRoot

    implicitWidth: mouseArea.implicitWidth
    implicitHeight: mouseArea.implicitHeight

    hoverEnabled: true

    stepSize: 0.02
    interactive: !Kirigami.Settings.hasTransientTouchInput

    // Workaround for https://bugreports.qt.io/browse/QTBUG-106118
    Binding on visible {
        delayed: true
        restoreMode: Binding.RestoreBindingOrValue
        value: controlRoot.size < 1.0 && controlRoot.size > 0 && controlRoot.policy !== T.ScrollBar.AlwaysOff && controlRoot.parent !== null
    }
    topPadding: style.topScrollbarPadding
    leftPadding: style.leftScrollbarPadding
    rightPadding: style.rightScrollbarPadding
    bottomPadding: style.bottomScrollbarPadding

    onPositionChanged: {
        if (handleGraphics.visible) {
            disappearTimer.restart();
            handleGraphics.handleState = Math.min(1, handleGraphics.handleState + 0.1)
        }
    }

    contentItem: Item {
        visible: !controlRoot.interactive

        Rectangle {
            id: handleGraphics

            // Controls auto-hide behavior state, 0 = hidden, 1 = fully visible
            property real handleState: 0

            x: controlRoot.vertical
                ? Math.round(!controlRoot.mirrored
                    ? (parent.width - width) - (parent.width/2 - width/2) * handleState
                    : (parent.width/2 - width/2) * handleState)
                : 0

            y: controlRoot.horizontal
                ? Math.round((parent.height - height) - (parent.height/2 - height/2) * handleState)
                : 0

            NumberAnimation on handleState {
                id: resetAnim
                from: handleGraphics.handleState
                to: 0
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
                // Same trick as in BusyIndicator. Animations using property
                // interceptor syntax are running by default. We don't want
                // this, as we will only run it with restart() method when needed.
                running: false
            }

            width: Math.round(controlRoot.vertical
                    ? Math.max(2, Kirigami.Units.smallSpacing * handleState)
                    : parent.width)
            height: Math.round(controlRoot.horizontal
                    ? Math.max(2, Kirigami.Units.smallSpacing * handleState)
                    : parent.height)
            radius: Math.min(width, height)
            color: Kirigami.Theme.textColor
            opacity: 0.3
            Timer {
                id: disappearTimer
                interval: 1000
                onTriggered: {
                    resetAnim.restart();
                }
            }
        }
    }

    background: MouseArea {
        id: mouseArea

        // The same as contentItem, but they are mutually exclusive anyway.
        // The stacking order needs to be greater than zero, so that it can
        // actually override ScrollBar's own internal event handling.
        // BUG: 488092
        z: 1

        anchors.fill: parent
        visible: controlRoot.size < 1.0 && controlRoot.interactive
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        onExited: style.activeControl = "groove";
        onPressed: mouse => {
            const jumpPosition = style.positionFromMouse(mouse);
            if (mouse.buttons & Qt.MiddleButton) {
                style.activeControl = "handle";
                controlRoot.position = jumpPosition;
                mouse.accepted = true;
            } else if (style.activeControl === "down") {
                buttonTimer.increment = 1;
                buttonTimer.running = true;
                mouse.accepted = true;
            } else if (style.activeControl === "up") {
                buttonTimer.increment = -1;
                buttonTimer.running = true;
                mouse.accepted = true;
            } else if (style.activeControl === "downPage") {
                if (style.scrollToClickPosition(mouse)) {
                    // Let the QQuickScrollBar handle it
                    mouse.accepted = false;
                } else {
                    buttonTimer.increment = controlRoot.size;
                    buttonTimer.running = true;
                    mouse.accepted = true;
                }
            } else if (style.activeControl === "upPage") {
                if (style.scrollToClickPosition(mouse)) {
                    // Let the QQuickScrollBar handle it
                    mouse.accepted = false;
                } else {
                    buttonTimer.increment = -controlRoot.size;
                    buttonTimer.running = true;
                    mouse.accepted = true;
                }
            } else {
                mouse.accepted = false;
            }
        }
        onPositionChanged: mouse => {
            style.activeControl = style.hitTest(mouse.x, mouse.y);
            if (mouse.buttons & Qt.MiddleButton) {
                style.activeControl = "handle";
                controlRoot.position = style.positionFromMouse(mouse);
                mouse.accepted = true;
            } else {
                // Let the QQuickScrollBar handle it
                mouse.accepted = false;
            }
        }
        onReleased: mouse => {
            style.activeControl = style.hitTest(mouse.x, mouse.y);
            buttonTimer.running = false;
            mouse.accepted = false;
        }
        onCanceled: buttonTimer.running = false;

        implicitWidth: style.implicitWidth
        implicitHeight: style.implicitHeight

        Timer {
            id: buttonTimer
            property real increment
            repeat: true
            interval: 150
            triggeredOnStart: true
            onTriggered: {
                if (increment === 1) {
                    controlRoot.increase();
                } else if (increment === -1) {
                    controlRoot.decrease();
                } else {
                    controlRoot.position = Math.min(1 - controlRoot.size, Math.max(0, controlRoot.position + increment));
                }
            }
        }
        StylePrivate.StyleItem {
            id: style

            readonly property real length: controlRoot.vertical ? height : width
            property rect grooveRect: Qt.rect(0, 0, 0, 0)
            readonly property real topScrollbarPadding: grooveRect.top
            readonly property real bottomScrollbarPadding: height - grooveRect.bottom
            readonly property real leftScrollbarPadding: grooveRect.left
            readonly property real rightScrollbarPadding: width - grooveRect.right

            onWidthChanged: computeRects()
            onHeightChanged: computeRects()
            onStyleNameChanged: computeRects()
            Component.onCompleted: computeRects()

            function computeRects() {
                grooveRect = subControlRect("groove");
            }

            // TODO Enable type annotation once we depend on Qt 6.7
            function positionFromMouse(mouse/*: MouseEvent*/): real {
                return Math.min(1 - controlRoot.size, Math.max(0,
                    (controlRoot.horizontal
                        ? mouse.x / width
                        : mouse.y / height
                    ) - controlRoot.size / 2
                ));
            }

            // Style hint returns true if it should scroll to click position,
            // and false if it should scroll by one page at a time.
            // This function inverts the behavior if Alt button is pressed.
            // TODO Enable type annotation once we depend on Qt 6.7
            function scrollToClickPosition(mouse/*: MouseEvent*/): bool {
                let behavior = style.styleHint("scrollToClickPosition");
                if (mouse.modifiers & Qt.AltModifier) {
                    behavior = !behavior;
                }
                return behavior;
            }

            control: controlRoot
            anchors.fill: parent
            elementType: "scrollbar"
            hover: activeControl !== "none"
            activeControl: "none"
            sunken: controlRoot.pressed
            // Normally, min size should be controlled by
            // PM_ScrollBarSliderMin pixel metrics, or ScrollBar.minimumSize
            // property. But we are working with visual metrics (0..1) here;
            // and despite what documentation says, minimumSize does not
            // affect visualSize. So let us at least prevent division by zero.
            minimum: 0
            maximum: Math.round(length / Math.max(0.001, controlRoot.visualSize) - length)
            value: Math.round(length / Math.max(0.001, controlRoot.visualSize) * Math.min(1 - 0.001, controlRoot.visualPosition))

            horizontal: controlRoot.horizontal
            enabled: controlRoot.enabled

            visible: controlRoot.size < 1.0
            opacity: 1
        }
        StylePrivate.StyleItem {
            id: inactiveStyle
            anchors.fill: parent
            control: controlRoot
            elementType: "scrollbar"
            activeControl: "none"
            sunken: false
            minimum: 0
            maximum: style.maximum
            value: style.value
            horizontal: style.horizontal
            enabled: controlRoot.enabled

            visible: controlRoot.size < 1.0
            opacity: 1
        }
        state: "inactive"
        states: [
            State {
                name: "hover"
                when: mouseArea.containsMouse
                PropertyChanges {
                    target: style
                    opacity: 1
                }
                PropertyChanges {
                    target: inactiveStyle
                    opacity: 0
                }
            },
            State {
                name: "inactive"
                when: !mouseArea.containsMouse
                PropertyChanges {
                    target: style
                    opacity: 0
                }
                PropertyChanges {
                    target: inactiveStyle
                    opacity: 1
                }
            }
        ]
        transitions: Transition {
            NumberAnimation {
                targets: [style, inactiveStyle]
                property: "opacity"
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
}
