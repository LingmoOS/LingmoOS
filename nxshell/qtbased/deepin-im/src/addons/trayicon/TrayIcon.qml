// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import Qt.labs.platform 1.1

SystemTrayIcon {
    visible: true
    icon.name: trayicon.imAddonIcon
    tooltip : qsTr("deepin input method trayicon")
    icon.source: "image://icon/" + trayicon.imAddonIcon

    menu: Menu {
        id: contextMenu

        Instantiator {
            model: trayicon.imEntries

            delegate: MenuItem {
                text: model.description
                icon.source: "image://icon/" + model.icon
                onTriggered: trayicon.imEntryMenuTriggered(model.addon, model.name)
            }

            onObjectAdded: function(index, object) {
                contextMenu.insertItem(index, object)
            }
            onObjectRemoved: function(index, object) {
                contextMenu.removeItem(object)
            }
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Settings")
            onTriggered: trayicon.configureTriggered()
        }
    }

    onActivated: {
        if (reason === SystemTrayIcon.Trigger) {
            trayicon.toggle();
        }
    }
}
