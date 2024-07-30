    /*
 *  SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_showTitle: showTitle.checked
    property alias cfg_title: title.text

    property alias cfg_showRemainingTime: showRemainingTime.checked
    property alias cfg_showSeconds: showSeconds.checked
    property alias cfg_showTimerToggle: showTimerToggle.checked
    property alias cfg_showProgressBar: showProgressBar.checked

    property alias cfg_showNotification: showNotification.checked
    property alias cfg_notificationText: notificationText.text

    Kirigami.FormLayout {
        RowLayout {
            Layout.fillWidth: true

            Kirigami.FormData.label: i18nc("@title:label", "Display:")

            QQC2.CheckBox {
                id: showTitle

                text: i18nc("@option:check", "Show title:");
                onClicked: {
                    if (checked) {
                        title.forceActiveFocus();
                    }
                }
            }

            QQC2.TextField {
                id: title
                Layout.fillWidth: true
                enabled: showTitle.checked
            }
        }

        QQC2.CheckBox {
            id: showRemainingTime
            text: i18nc("@option:check", "Show remaining time");
        }

        QQC2.CheckBox {
            id: showSeconds
            enabled: showRemainingTime.checked
            text: i18nc("@option:check", "Show seconds");
        }

        QQC2.CheckBox {
            id: showTimerToggle
            text: i18nc("@option:check", "Show timer toggle");
        }

        QQC2.CheckBox {
            id: showProgressBar
            text: i18nc("@option:check", "Show progress bar");
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Layout.fillWidth: true

            Kirigami.FormData.label: i18nc("@title:label", "Notifications:")

            QQC2.CheckBox {
                id: showNotification
                text: i18nc("@option:check", "Show notification text:");
                onClicked: {
                    if (checked) {
                        notificationText.forceActiveFocus();
                    }
                }
            }

            QQC2.TextField {
                id: notificationText
                Layout.fillWidth: true
                enabled: showNotification.checked
            }
        }
    }
}

