/*
    SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCMUtils
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.networkmanagement as LingmoNM

QQC2.ApplicationWindow {
    id: root

    required property LingmoNM.Handler handler

    title: i18nc("@title:window", "Configuration")

    minimumHeight: LingmoUI.Units.gridUnit * 6
    maximumHeight: page.implicitHeight + dialogButtonBox.implicitHeight
    height: maximumHeight
    minimumWidth: LingmoUI.Units.gridUnit * 15
    width: LingmoUI.Units.gridUnit * 25

    KCMUtils.SimpleKCM {
        id: page

        anchors.fill: parent

        LingmoUI.FormLayout {
            LingmoUI.Separator {
                LingmoUI.FormData.label: i18n("General")
                LingmoUI.FormData.isSection: true
            }

            QQC2.CheckBox {
                id: unlockModem
                Layout.fillWidth: true
                KeyNavigation.down: manageVirtualConnections
                text: i18n("Ask for PIN on modem detection")
            }

            QQC2.CheckBox {
                id: manageVirtualConnections
                Layout.fillWidth: true
                KeyNavigation.down: hotspotName
                text: i18n("Show virtual connections")
            }

            LingmoUI.Separator {
                id: hotspotLabel
                LingmoUI.FormData.label: i18n("Hotspot")
                LingmoUI.FormData.isSection: true
            }

            QQC2.TextField {
                id: hotspotName
                Layout.fillWidth: true
                KeyNavigation.down: hotspotPassword
                LingmoUI.FormData.label: i18nc("@label Hotspot name", "Name:")
            }

            LingmoUI.PasswordField {
                id: hotspotPassword
                Layout.fillWidth: true
                LingmoUI.FormData.label: i18nc("@label Hotspot password", "Password:")
                showPassword: true
                validator: RegularExpressionValidator {
                    // useApMode is a context property
                    regularExpression: useApMode
                        ? /^$|^(?:.{8,64}){1}$/
                        : /^$|^(?:.{5}|[0-9a-fA-F]{10}|.{13}|[0-9a-fA-F]{26}){1}$/
                }
            }
        }

        footer: ColumnLayout {
            spacing: 0

            LingmoUI.InlineMessage {
                Layout.fillWidth: true
                Layout.margins: LingmoUI.Units.largeSpacing
                Layout.bottomMargin: 0
                type: LingmoUI.MessageType.Error
                text: i18n("Hotspot name must not be empty")
                visible: handler.hotspotSupported && hotspotName.text === ""
            }

            LingmoUI.InlineMessage {
                Layout.fillWidth: true
                Layout.margins: LingmoUI.Units.largeSpacing
                Layout.bottomMargin: 0
                type: LingmoUI.MessageType.Error
                text: useApMode
                    ? i18n("Hotspot password must either be empty or consist of anywhere from 8 up to 64 characters")
                    : i18n("Hotspot password must either be empty or consist of one of the following:<ul><li>Exactly 5 or 13 any characters</li><li>Exactly 10 or 26 hexadecimal characters:<br/>abcdef, ABCDEF or 0-9</li></ul>")
                visible: handler.hotspotSupported && !hotspotPassword.acceptableInput
            }
        }
    }

    footer: QQC2.DialogButtonBox {
        id: dialogButtonBox

        standardButtons: QQC2.DialogButtonBox.Ok | QQC2.DialogButtonBox.Cancel | QQC2.DialogButtonBox.Reset

        onAccepted: root.accept()
        onRejected: root.reject()
        onReset: root.loadConfiguration()

        KeyNavigation.up: hotspotPassword

        Component.onCompleted: {
            const okButton = standardButton(QQC2.DialogButtonBox.Ok);
            if (okButton) {
                okButton.enabled = Qt.binding(() => root.acceptableConfiguration());
                hotspotPassword.KeyNavigation.down = okButton;
            }
        }
    }

    Shortcut {
        sequences: ["Return", "Enter"]
        onActivated: root.accept()
    }

    Shortcut {
        sequences: [StandardKey.Back, StandardKey.Cancel, StandardKey.Close]
        onActivated: root.reject()
    }

    // Unfortunately, unlike Dialog, DialogButtonBox lacks accept and reject
    // methods. Though unlike DialogButtonBox we also want to make sure that
    // OK button is enabled.
    function accept() {
        if (acceptableConfiguration()) {
            saveConfiguration();
            close();
        }
    }

    function reject() {
        close();
    }

    function acceptableConfiguration(): bool {
        if (handler.hotspotSupported) {
            return hotspotName.text !== "" && hotspotPassword.acceptableInput;
        } else {
            return true;
        }
    }

    function loadConfiguration() {
        unlockModem.checked = LingmoNM.Configuration.unlockModemOnDetection;
        manageVirtualConnections.checked = LingmoNM.Configuration.manageVirtualConnections;
        // hotspot
        hotspotLabel.visible = handler.hotspotSupported;
        hotspotName.visible = handler.hotspotSupported;
        hotspotPassword.visible = handler.hotspotSupported;
        if (handler.hotspotSupported) {
            hotspotName.text = LingmoNM.Configuration.hotspotName;
            hotspotPassword.text = LingmoNM.Configuration.hotspotPassword;
        }
    }

    function saveConfiguration() {
        LingmoNM.Configuration.unlockModemOnDetection = unlockModem.checked;
        LingmoNM.Configuration.manageVirtualConnections = manageVirtualConnections.checked;
        if (handler.hotspotSupported) {
            LingmoNM.Configuration.hotspotName = hotspotName.text;
            LingmoNM.Configuration.hotspotPassword = hotspotPassword.text;
        }
    }

    onVisibleChanged: {
        if (visible) {
            loadConfiguration();
            unlockModem.forceActiveFocus(Qt.ActiveWindowFocusReason);
        }
    }
}
