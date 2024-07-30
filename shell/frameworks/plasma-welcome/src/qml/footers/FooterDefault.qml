/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

RowLayout {
    id: root

    spacing: Kirigami.Units.smallSpacing

    readonly property bool inLayer: pageStack.layers.depth > 1
    readonly property bool atStart: pageStack.currentIndex === 0
    readonly property bool atEnd: pageStack.currentIndex === pageStack.depth - 1

    QQC2.Button {
        Layout.alignment: Qt.AlignLeft

        action: Kirigami.Action {
            readonly property bool isSkip: root.atStart && !root.inLayer

            text: isSkip ? i18nc("@action:button", "&Skip") : i18nc("@action:button", "&Back")
            icon.name: {
                if (isSkip) {
                    return "dialog-cancel-symbolic";
                } else if (Qt.application.layoutDirection === Qt.LeftToRight) {
                    "go-previous-symbolic"
                } else {
                    "go-previous-rtl-symbolic"
                }
            }
            shortcut: Qt.application.layoutDirection === Qt.LeftToRight ? "Left" : "Right"

            onTriggered: {
                if (root.inLayer) {
                    pageStack.layers.pop();
                } else if (!root.atStart) {
                    pageStack.currentIndex -= 1;
                } else {
                    Qt.quit();
                }
            }
        }
    }

    QQC2.PageIndicator {
        Layout.alignment: Qt.AlignHCenter

        enabled: !root.inLayer
        count: pageStack.depth
        currentIndex: pageStack.currentIndex
        onCurrentIndexChanged: pageStack.currentIndex = currentIndex
        interactive: true
    }

    QQC2.Button {
        id: nextButton
        Layout.alignment: Qt.AlignRight

        enabled: !root.inLayer

        action: Kirigami.Action {
            text: root.atEnd ? i18nc("@action:button", "&Finish") : i18nc("@action:button", "&Next")
            icon.name: {
                if (root.atEnd) {
                    return "dialog-ok-apply-symbolic";
                } else if (Qt.application.layoutDirection === Qt.LeftToRight) {
                    "go-next-symbolic"
                } else {
                    "go-next-rtl-symbolic"
                }
            }
            shortcut: Qt.application.layoutDirection === Qt.LeftToRight ? "Right" : "Left"

            enabled: nextButton.enabled

            onTriggered: {
                if (!root.atEnd) {
                    pageStack.currentIndex += 1;
                } else {
                    Qt.quit();
                }
            }
        }
    }
}
