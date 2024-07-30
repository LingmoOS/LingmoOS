/**
 * SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PM

Kirigami.Dialog {

    title: i18nc("@title:window", "Printer Setup Helpers")

    closePolicy: installer.installing
                 ? QQC2.Popup.NoAutoClose
                 : QQC2.Popup.CloseOnEscape | QQC2.Popup.CloseOnReleaseOutside
    standardButtons: Kirigami.Dialog.NoButton

    padding: Kirigami.Units.largeSpacing

    Component.onCompleted: {
        open()
        installer.install()
    }

    onClosed: {
        if (installer.installed) {
            scpMessage.visible = false
        }

        scpLoader.active = false
    }

    PM.SCPInstaller {
        id: installer

        onError: errMsg => {
                     statusMsg.text = errMsg
                 }
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            source: {
                if (installer.installing) {
                    "install"
                } else if (installer.installed) {
                    "installed"
                } else {
                    "error"
                }
            }
        }

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            QQC2.Label {
                id: statusMsg
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: {
                    if (installer.installed) {
                        i18nc("@info:status", "Install has completed successfully")
                    } else if (installer.failed) {
                        i18nc("@info:status", "Installation failed")
                    } else if (installer.installing) {
                        i18nc("@info:status", "Installing…")
                    } else {
                        ""
                    }
                }
            }

            QQC2.ProgressBar {
                Layout.fillWidth: true
                indeterminate: true
                visible: installer.installing
            }

            Kirigami.UrlButton {
                Layout.topMargin: Kirigami.Units.largeSpacing
                visible: installer.failed
                text: i18nc("@info:usagetip", "Report this issue to %1", kcm.osName)
                url: kcm.osBugReportUrl
            }
        }
    }
}
