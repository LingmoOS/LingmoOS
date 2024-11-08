/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Kate Leet <kateleet@lingmoos.com>
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
    headerTitle: qsTr("Notifications")

    Notifications {
        id: notifications
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            RoundedItem {
                RowLayout {
                    Label {
                        text: qsTr("Do Not Disturb")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        checked: notifications.doNotDisturb
                        Layout.fillHeight: true
                        onClicked: notifications.doNotDisturb = checked
                    }
                }
            }
        }
    }
}
