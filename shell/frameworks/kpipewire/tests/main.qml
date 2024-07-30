/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1
import org.kde.kirigami 2.15 as Kirigami
import org.kde.taskmanager

import org.kde.pipewire 0.1 as PipeWire

Kirigami.ApplicationWindow
{
    id: root
    width: 500
    height: 500
    visible: true
    property QtObject app

    function addStream(nodeid, displayText, fd, allowDmaBuf) {
        if (fd == null)
            fd = 0;
        rep.model.append({nodeId: nodeid, uuid: "", display: displayText, fd: fd, allowDmaBuf: allowDmaBuf })
    }
    function removeStream(nodeid) {
        for(var i=0; i<rep.model.count; ++i) {
            if (rep.model.get(i).nodeId === nodeid) {
               rep.model.remove(i)
                break;
            }
        }
    }

    Instantiator {
        model: TasksModel {
            groupMode: TasksModel.GroupDisabled
        }
        delegate: Item {
            property var uuid: model.WinIdList
            property var appId: model.AppId
        }
        onObjectAdded: (index, object) => {
           app.addWindow(object.uuid, object.appId)
        }
    }

    ColumnLayout {
        id: pipes
        anchors.fill: parent

        Button {
            text: "Add Virtual Monitor"
            onClicked: app.createVirtualMonitor()
        }

        Button {
            text: "Add Region"
            onClicked: app.requestSelection()
        }

        ListView {
            id: rep

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            model: ListModel {}
            delegate: RowLayout {
                Kirigami.Icon {
                    source: sourceItem.usingDmaBuf ? "speedometer" : "delete"
                }

                Button {
                    text: sourceItem.visible ? "Pause" : "Resume"
                    icon.name: sourceItem.visible ? "media-playback-pause" : "media-playback-start"
                    onClicked: sourceItem.visible = !sourceItem.visible;
                }

                Label {
                    text: model.display
                }

                PipeWire.PipeWireSourceItem {
                    id: sourceItem

                    Layout.preferredWidth: Kirigami.Units.gridUnit * 16
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 8

                    nodeId: model.nodeId
                    fd: model.fd
                    allowDmaBuf: model.allowDmaBuf
                }
            }
        }
    }
}
