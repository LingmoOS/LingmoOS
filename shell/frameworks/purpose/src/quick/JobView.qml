/*
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.purpose
import org.kde.kirigami as Kirigami

Item {

    property alias model: jobController.model
    property alias index: jobController.index

    readonly property alias state: jobController.state
    readonly property alias job: jobController.job

    implicitHeight: mainLoader.implicitHeight

    function start() {
        jobController.configure()
    }

    PurposeJobController {
        id: jobController
    }

    Loader {
        id: mainLoader
        anchors.fill: parent

        sourceComponent: {
            switch (jobController.state) {
            case PurposeJobController.Configuring:
                return configuring;
            case PurposeJobController.Running:
                return running;
            default:
                return null;
            }
        }
    }

    Component {
        id: running

        Item {
            BusyIndicator {
                running: true
                anchors.centerIn: parent
            }
        }
    }

    Component {
        id: configuring

        ColumnLayout {
            Loader {
                id: configLoader

                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.largeSpacing

                Layout.fillHeight: true
                Layout.fillWidth: true

                Component.onCompleted: setSource(jobController.configuration.configSourceCode, jobController.configuration.data)

                onItemChanged: {
                    var initialData = jobController.configuration.data;
                    for (var i in jobController.configuration.neededArguments) {
                        var arg = jobController.configuration.neededArguments[i];
                        if (arg in configLoader.item) {
                            item[arg + "Changed"].connect(dataHasChanged);
                            initialData[arg] = item[arg];
                        } else {
                            console.warn("property not found", arg);
                        }
                    }
                    jobController.configuration.data = initialData;
                }

                function dataHasChanged() {
                    var jobData = jobController.configuration.data;
                    for (var i in jobController.configuration.neededArguments) {
                        var arg = jobController.configuration.neededArguments[i];
                        if (arg in configLoader.item) {
                            jobData[arg] = configLoader.item[arg];
                        } else
                            console.warn("property not found", arg);
                    }
                    jobController.configuration.data = jobData;
                }
            }

            Label {
                textFormat: Text.PlainText
                Layout.fillWidth: true
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                visible: configLoader.status === Loader.Error
                wrapMode: Text.WordWrap
                text: configLoader.status === Loader.Error
                    ? i18nd("libpurpose6_quick", "Failed to load the configuration page for this action:\n\n%1", configLoader.sourceComponent.errorString())
                    : ""
            }

            // Not using a DialogButtonBox because it doesn't let us customize
            // the buttons and conditionally disable any of them, which we want
            RowLayout {
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                Layout.alignment: Qt.AlignRight

                Button {
                    text: i18nd("libpurpose6_quick", "Send")
                    icon.name: "document-send"
                    enabled: jobController.configuration
                        && jobController.configuration.isReady
                    onClicked: jobController.startJob()
                }

                Button {
                    text: i18nd("libpurpose6_quick", "Cancel")
                    icon.name: "dialog-cancel"
                    onClicked: jobController.cancel()
                }
            }
        }
    }
}
