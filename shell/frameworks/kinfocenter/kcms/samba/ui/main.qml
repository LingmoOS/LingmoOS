// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

import org.kde.kcmutils as KCM
import QtQuick 2.14
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.lingmoui.delegates as KD
import QtQuick.Controls 2.14 as QQC2
import QtQuick.Layouts 1.14
import org.kde.kinfocenter.samba 1.0 as Samba

KCM.AbstractKCM {
    GridLayout {
        anchors.fill: parent
        columns: 2

        LingmoUI.Heading {
            text: i18nc("@title heading above listview", "User-Created Shares")
            level: 2
        }

        LingmoUI.Heading {
            text: i18nc("@title heading above listview", "Mounted Remote Shares")
            level: 2
        }

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            LingmoUI.Theme.colorSet: LingmoUI.Theme.View
            LingmoUI.Theme.inherit: false
            Component.onCompleted: background.visible = true // crashes when initialized with this. god knows why

            ListView {
                id: view
                keyNavigationEnabled: false
                model: Samba.ShareModel{}

                delegate: ShareListItem {
                    width: view.width

                    // The view isn't navigatable nor interactable. Disable highlighting.
                    highlighted: false
                    hoverEnabled: false
                    down: false
                }

                LingmoUI.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (LingmoUI.Units.largeSpacing * 4)
                    visible: parent.count == 0
                    icon.name: "network-server"
                    text: i18nc("@info place holder for empty listview", "There are no directories shared by users")
                }
            }
        }

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            LingmoUI.Theme.colorSet: LingmoUI.Theme.View
            LingmoUI.Theme.inherit: false
            Component.onCompleted: background.visible = true // crashes when initialized with this. god knows why

            ListView {
                currentIndex: -1
                model: Samba.MountModel {}

                delegate: KD.SubtitleDelegate {
                    // TODO document-open-remote is actually pretty cool but lacks a visualization for not connected
                    //   emblem icons are kind of a crutch
                    icon.name: ROLE_Accessible ? "emblem-mounted" : "emblem-unmounted"
                    text: ROLE_Path
                    subtitle: ROLE_Share
                    onClicked: {
                        // Append a slash as openurlexternally fucks with perfectly valid urls and turns them into
                        // invalid ones (file:///srv => file://srv) that KIO then thinks is a windows UNC path
                        // (file://srv => smb://srv).
                        // By appending a slash we effectively trick Qt. Kinda meh.
                        Qt.openUrlExternally("file://" + ROLE_Path + "/")
                    }
                }

                LingmoUI.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (LingmoUI.Units.largeSpacing * 4)
                    visible: parent.count == 0
                    icon.name: "folder-network"
                    text: i18nc("@info place holder for empty listview",
                                "There are no Samba shares mounted on this system")
                }
            }
        }
    }
}
