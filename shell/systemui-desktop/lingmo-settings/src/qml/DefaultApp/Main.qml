/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Kate Leet <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import LingmoUI 1.0 as LingmoUI
import Lingmo.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Default Applications")

    DefaultApplications {
        id: defaultApps
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            RoundedItem {
                GridLayout {
                    columns: 2
                    columnSpacing: LingmoUI.Units.largeSpacing * 2

                    Label {
                        text: qsTr("Web Browser")
                        enabled: browserComboBox.count !== 0
                    }

                    AppComboBox {
                        id: browserComboBox
                        Layout.fillWidth: true
                        textRole: "name"
                        model: defaultApps.browserList
                        currentIndex: defaultApps.browserIndex
                        enabled: count !== 0
                        onActivated: {
                            defaultApps.setDefaultBrowser(browserComboBox.currentIndex)
                        }
                    }

                    Label {
                        text: qsTr("File Manager")
                        enabled: fileManagerComboBox.count !== 0
                    }

                    AppComboBox {
                        id: fileManagerComboBox
                        Layout.fillWidth: true
                        textRole: "name"
                        model: defaultApps.fileManagerList
                        currentIndex: defaultApps.fileManagerIndex
                        enabled: count !== 0
                        onActivated: {
                            defaultApps.setDefaultFileManager(fileManagerComboBox.currentIndex)
                        }
                    }

                    Label {
                        text: qsTr("Email Client")
                        enabled: emailComboBox.count !== 0
                    }

                    AppComboBox {
                        id: emailComboBox
                        Layout.fillWidth: true
                        textRole: "name"
                        model: defaultApps.emailList
                        currentIndex: defaultApps.emailIndex
                        enabled: count !== 0
                        onActivated: {
                            defaultApps.setDefaultEMail(emailComboBox.currentIndex)
                        }
                    }

                    Label {
                        text: qsTr("Terminal")
                        enabled: terminalComboBox.count !== 0
                    }

                    AppComboBox {
                        id: terminalComboBox
                        Layout.fillWidth: true
                        textRole: "name"
                        model: defaultApps.terminalList
                        currentIndex: defaultApps.terminalIndex
                        enabled: count !== 0
                        onActivated: {
                            defaultApps.setDefaultTerminal(terminalComboBox.currentIndex)
                        }
                    }
                }
            }
        }
    }
}
