/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.pipewire.monitor as PWMonitor
import org.kde.pipewire as PipeWire
import org.kde.pipewire.record as PWR

Kirigami.ApplicationWindow
{
    id: root
    width: 500
    height: 500
    visible: true
    property QtObject app

    pageStack.initialPage: Kirigami.Page {
        ColumnLayout {
            anchors.fill: parent

            Repeater {
                id: rep
                model: PWMonitor.MediaMonitor {
                    role: PWMonitor.MediaMonitor.Camera
                }

                delegate: Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    PipeWire.PipeWireSourceItem {
                        nodeId: model.objectSerial
                        fd: model.fd
                        anchors.fill: parent
                    }

                    Label {
                        text: model.display + " xxx " + model.objectSerial
                    }
                    PWR.PipeWireRecord {
                        id: rec
                        nodeId: model.objectSerial
                        fd: model.fd
                        active: recButton.checked
                        output: "/tmp/banana.webm"
                    }
                    Button {
                        id: recButton
                        icon.name: "media-record"
                        checkable: true
                    }
                }
            }
        }
    }
}
