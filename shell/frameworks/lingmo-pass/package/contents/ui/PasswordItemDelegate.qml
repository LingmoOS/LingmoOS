// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0

import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.components 2.0 as LingmoComponents
import org.kde.lingmo.components 3.0 as LingmoComponents3

import org.kde.lingmo.private.lingmopass 1.0

LingmoComponents.ListItem {
    id: root

    property alias name: label.text
    property string icon
    property var entryType

    property PasswordProvider passwordProvider: null
    property OTPProvider otpProvider: null

    property alias provider: root.passwordProvider

    signal itemSelected(var index)
    signal otpClicked(var index)

    enabled: true

    implicitHeight: Math.max(column.height, otpButton.implicitHeight + 2 * LingmoCore.Units.smallSpacing)

    onClicked: {
        root.itemSelected(index);
    }

    onContainsMouseChanged: {
        if (containsMouse) {
            listView.currentIndex = index
        } else {
            listView.currentIndex = -1
        }
    }

    // When password becomes invalid again, forget about it
    Connections {
        property bool wasValid : false
        target: root.provider
        onValidChanged: {
            if (wasValid && !target.valid) {
                root.provider = null;
            } else if (!wasValid && target.valid) {
                wasValid = true;
                // Password has become valid, wait a little bit and then close the plasmoid
                hideTimer.start()
            }
        }
    }

    Timer {
        id: hideTimer
        interval: LingmoCore.Units.longDuration
        onTriggered: plasmoid.expanded = false;
    }

    Column {
        id: column
        spacing: LingmoCore.Units.smallSpacing
        anchors {
            left: parent.left
            right: parent.right
        }

        RowLayout {
            spacing: LingmoCore.Units.largeSpacing
            id: row

            width: parent.width

            LingmoCore.IconItem {
                id: entryTypeIcon
                visible: root.provider == null || root.provider.valid || root.provider.hasError
                source: {
                    if (root.provider == null) {
                        return root.icon;
                    } else {
                        if (root.provider.hasError) {
                            return "dialog-error";
                        } else {
                            return "dialog-ok";
                        }
                    }
                }
                width: LingmoCore.Units.iconSizes.small
                height: LingmoCore.Units.iconSizes.small
            }

            LingmoComponents3.BusyIndicator {
                id: busyIndicator
                visible: root.provider != null && !root.provider.valid && !root.provider.hasError

                // Hack around BI wanting to be too large by default
                Layout.maximumWidth: entryTypeIcon.width
                Layout.maximumHeight: entryTypeIcon.height
            }

            LingmoComponents3.Label {
                id: label

                height: undefined // unset LingmoComponents.Label default height

                Layout.fillWidth: true

                maximumLineCount: 1
                verticalAlignment: Text.AlignLeft
                elide: Text.ElideRight
                textFormat: Text.PlainText
            }

            LingmoComponents3.ToolButton {
                id: otpButton
                icon.name: 'clock'
                visible: entryType == PasswordsModel.PasswordEntry

                LingmoComponents3.ToolTip {
                    text: i18n("One-time password (OTP)")
                }

                onClicked: {
                    root.otpClicked(index);
                }
            }
        }

        ProviderDelegate {
            id: passwordDelegate
            provider: root.passwordProvider
            icon: 'lock'
            visible: root.passwordProvider !== null
            width: parent.width
        }

        ProviderDelegate {
            id: otpDelegate
            provider: root.otpProvider
            icon: 'clock'
            visible: root.otpProvider !== null
            width: parent.width
        }
    }
}

