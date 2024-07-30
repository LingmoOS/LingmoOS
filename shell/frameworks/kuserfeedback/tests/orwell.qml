/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Qt.labs.settings 1.0
import org.kde.userfeedback 1.0 as KUserFeedback

ApplicationWindow {
    id: root
    width: 480
    height: 640

    Component.onCompleted: {
        Qt.application.name = "qmlorwell";
        Qt.application.version = "1984.qml"
        Qt.application.organization = "KDE";
        Qt.application.domain = "kde.org";

        // for Settings and thus Provider to work, we need to have the above setup first
        stackView.push(mainView);
    }

    StackView {
        id: stackView
        anchors.fill: parent
    }

    Component {
        id: mainView
        Page {
            header: ToolBar {
                RowLayout {
                    anchors.fill: parent
                    Label {
                        text: "Feedback Demo"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        id: menuButton
                        text: "⋮"
                        onClicked: menu.open()
                        Menu {
                            id: menu
                            y: menuButton.height
                            MenuItem {
                                text: "Contribute..."
                                visible: provider.enabled
                                onTriggered: {
                                    stackView.push(contributePage);
                                    stackView.currentItem.provider = provider;
                                }
                            }
                        }
                    }
                }
            }

            KUserFeedback.Provider {
                id: provider
                submissionInterval: 1
                productIdentifier: "org.kde.orwell"
                feedbackServer: "https://feedback.volkerkrause.eu/"
                applicationStartsUntilEncouragement: 5
                encouragementDelay: 10
                encouragementInterval: 1

                onSurveyAvailable: {
                    console.log(survey);
                    surveyPopup.surveyInfo = survey;
                    surveyPopup.provider = provider;
                    surveyPopup.open();
                }
                onShowEncouragementMessage: {
                    console.log("showing encouragement");
                    if (stackView.depth == 1) {
                        encouragementPopup.provider = provider
                        encouragementPopup.open();
                    }
                }

                KUserFeedback.ApplicationVersionSource {}
                KUserFeedback.CompilerInfoSource {}
                KUserFeedback.CpuInfoSource {}
                KUserFeedback.LocaleInfoSource {}
                KUserFeedback.OpenGLInfoSource{}
                KUserFeedback.PlatformInfoSource {}
                KUserFeedback.QPAInfoSource {}
                KUserFeedback.QtVersionSource {}
                KUserFeedback.ScreenInfoSource {}
                KUserFeedback.StartCountSource {}
                KUserFeedback.UsageTimeSource {}

                KUserFeedback.PropertyRatioSource {
                    id: dialRatioSource
                    mode: KUserFeedback.Provider.DetailedUsageStatistics
                    sourceId: "dialRatio"
                    object: dial
                    propertyName: "intValue"
                    name: qsTr("Dial position")
                    description: qsTr("The position of the dial.")

                    Component.onCompleted: {
                        dialRatioSource.addValueMapping(0, "off");
                        dialRatioSource.addValueMapping(11, "max");
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                Dial {
                    property int intValue: Math.round(dial.value)
                    id: dial
                    Layout.fillWidth: true
                    from: 0
                    to: 11
                    stepSize: 1
                }
                Settings {
                    property alias dialValue: dial.value
                }
                Button {
                    id: submitButton
                    Layout.alignment: Qt.AlignHCenter
                    text: "Submit!"
                    onClicked: provider.submit()
                }
            }
        }
    }

    Component {
        id: contributePage
        Page {
            property var provider: null
            header: ToolBar {
                RowLayout {
                    anchors.fill: parent
                    ToolButton {
                        id: menuButton
                        text: "<"
                        onClicked: stackView.pop()
                    }
                    Label {
                        text: "Contribution Settings"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                    }
                }
            }

            KUserFeedback.FeedbackConfigUiController {
                id: controller
                feedbackProvider: provider
                onProviderChanged: {
                    telemetrySlider.to = controller.telemetryModeCount - 1;
                    telemetrySlider.value = controller.telemetryModeToIndex(provider.telemetryMode);
                    surveySlider.value = controller.surveyIntervalToIndex(provider.surveyInterval);
                }
            }

            KUserFeedback.AuditLogUiController {
                id: auditLogController
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: header.height

                Label {
                    text: qsTr("Contribute Statistics")
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.bold: true
                }
                Slider {
                    id: telemetrySlider
                    stepSize: 1
                    Layout.fillWidth: true
                    snapMode: Slider.SnapAlways
                }
                Label {
                    id: telemetryLabel
                    text: controller.telemetryModeDescription(telemetrySlider.value)
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                TextArea {
                    id: telemetryDetails
                    visible: telemetrySlider.value > 0
                    text: controller.telemetryModeDetails(telemetrySlider.value)
                    Layout.fillWidth: true
                    textFormat: Text.RichText
                    wrapMode: TextEdit.Wrap
                    readOnly: true
                }
                Label {
                    text: qsTr("<a href=\"auditLog\">View previously submitted data...</a>")
                    visible: auditLogController.hasLogEntries
                    onLinkActivated: {
                        stackView.push(auditLogPage);
                        stackView.currentItem.auditLogController = auditLogController
                    }
                }
                Label {
                    text: qsTr("Participate in Surveys")
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }

                Slider {
                    id: surveySlider
                    stepSize: 1
                    to: controller.surveyModeCount - 1
                    Layout.fillWidth: true
                    snapMode: Slider.SnapAlways
                }
                Label {
                    id: surveyLabel
                    text: controller.surveyModeDescription(surveySlider.value)
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: telemetrySlider.value + surveySlider.value === 0 ?
                        qsTr("I don't contribute.") :
                        qsTr("Contribute!");
                    onClicked: {
                        provider.telemetryMode = controller.telemetryIndexToMode(telemetrySlider.value);
                        provider.surveyInterval = controller.surveyIndexToInterval(surveySlider.value);
                        stackView.pop();
                    }
                }
            }
        }
    }

    Component {
        id: auditLogPage
        Page {
            property var auditLogController: null
            header: ToolBar {
                RowLayout {
                    anchors.fill: parent
                    ToolButton {
                        id: menuButton
                        text: "<"
                        onClicked: stackView.pop()
                    }
                    Label {
                        text: "Telemetry Audit Log"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: header.height

                ComboBox {
                    id: logEntryBox
                    Layout.fillWidth: true
                    model: auditLogController.logEntryModel
                    textRole: "display"
                    onActivated: {
                        var index = model.index(currentIndex, 0);
                        logEntryView.text = auditLogController.logEntry(model.data(index, 256));
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    TextArea {
                        id: logEntryView
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                        readOnly: true
                    }
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Delete Log")
                    onClicked: {
                        auditLogController.clear();
                        stackView.pop();
                    }
                }
            }

            onAuditLogControllerChanged: {
                if (auditLogController == null)
                    return;
                var index = auditLogController.logEntryModel.index(0, 0);
                logEntryView.text = auditLogController.logEntry(auditLogController.logEntryModel.data(index, 256));
            }
        }
    }

    Popup {
        id: surveyPopup
        property var surveyInfo;
        property var provider;
        x: 0
        y: 0
        width: parent.width
        height: parent.height

        ColumnLayout {
            anchors.fill: parent
            Label {
                Layout.fillWidth: true
                horizontalAlignment: Qt.AlignHCenter;
                text: qsTr("We are looking for your feedback!")
                font.bold: true
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("We would like a few minutes of your time to provide feedback about this application in a survey.")
                wrapMode: Text.WordWrap
            }
            Button {
                Layout.alignment: Qt.AlignHCenter;
                text: qsTr("Participate!")
                onClicked: {
                    if (Qt.openUrlExternally(surveyPopup.surveyInfo.url))
                        provider.surveyCompleted(surveyPopup.surveyInfo);
                    surveyPopup.close()
                }
            }
        }
    }

    Popup {
        id: encouragementPopup
        property var provider;
        x: 0
        y: 0
        width: parent.width
        height: parent.height

        ColumnLayout {
            anchors.fill: parent
            Label {
                text: qsTr("Help us make this application better!");
                Layout.fillWidth: true
                horizontalAlignment: Qt.AlignHCenter;
                font.bold: true
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("You can help us improving this application by sharing statistics and participate in surveys.")
                wrapMode: Text.WordWrap
            }
            Button {
                Layout.alignment: Qt.AlignHCenter;
                text: qsTr("Contribute...")
                onClicked: {
                    encouragementPopup.close();
                    stackView.push(contributePage);
                    stackView.currentItem.provider = provider;
                }
            }
        }
    }
}
