/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.newstuff as NewStuff

Item {
    property QtObject newStuffModel

    visible: opacity > 0
    opacity: (model.entry.status === NewStuff.Entry.Installing || model.entry.status === NewStuff.Entry.Updating) ? 1 : 0

    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration } }
    Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
        opacity: 0.9
    }
    QQC2.BusyIndicator {
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.verticalCenter
            bottomMargin: Kirigami.Units.smallSpacing
        }
        running: parent.visible
    }
    QQC2.Label {
        id: statusLabel
        Connections {
            target: newStuffModel
            function onEntryChanged(entry) {
                const status = entry.status;
                if (status === NewStuff.Entry.Downloadable
                    || status === NewStuff.Entry.Installed
                    || status === NewStuff.Entry.Updateable
                    || status === NewStuff.Entry.Deleted) {
                    statusLabel.text = "";
                } else if (status === NewStuff.Entry.Installing) {
                    statusLabel.text = i18ndc("knewstuff6", "Label for the busy indicator showing an item is being installed OR uninstalled", "Working…");
                } else if (status === NewStuff.Entry.Updating) {
                    statusLabel.text = i18ndc("knewstuff6", "Label for the busy indicator showing an item is in the process of being updated", "Updating…");
                } else {
                    statusLabel.text = i18ndc("knewstuff6", "Label for the busy indicator which should only be shown when the entry has been given some unknown or invalid status.", "Invalid or unknown state. <a href=\"https://bugs.kde.org/enter_bug.cgi?product=frameworks-knewstuff\">Please report this to the KDE Community in a bug report</a>.");
                }
            }
        }
        onLinkActivated: link => Qt.openUrlExternally(link)
        anchors {
            top: parent.verticalCenter
            left: parent.left
            right: parent.right
            margins: Kirigami.Units.smallSpacing
        }
        horizontalAlignment: Text.AlignHCenter
        // TODO: This is where we'd want to put the download progress and cancel button as well
        text: i18ndc("knewstuff6", "Label for the busy indicator showing an item is installing", "Installing…")
    }
}
