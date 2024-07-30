// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.9 as Kirigami

import PicoWizard 1.0

Module {
    id: root

    delegate: Item {
        ColumnLayout {
            anchors {
                top: parent.top
                bottom: nextButton.top
                bottomMargin: 16
                horizontalCenter: parent.horizontalCenter
            }

            PlasmaComponents.TextField {
                id: searchText
                Layout.preferredWidth: root.width * 0.7
                topPadding: 16
                bottomPadding: 16
                placeholderText: qsTr("Timezone")
                inputMethodHints: Qt.ImhNoPredictiveText

                onTextChanged: {
                    timezoneModule.filterText = text
                }
            }

            Rectangle {
                id: tzContainer
                Layout.preferredWidth: root.width * 0.7
                Layout.fillHeight: true

                radius: 4
                border.width: 2
                border.color: Kirigami.Theme.backgroundColor
                color: Qt.lighter(Kirigami.Theme.backgroundColor, 1.1)

                ScrollIndicator {
                    id: tzScroll
                    width: 12
                    anchors {
                        top: tzContainer.top
                        right: tzContainer.right
                        bottom: tzContainer.bottom
                    }
                }

                ListView {
                    id: tzListView
                    anchors.fill: parent
                    anchors.margins: 8

                    spacing: 4
                    model: timezoneModule.model
                    clip: true
                    ScrollIndicator.vertical: tzScroll

                    delegate: Rectangle {
                        width: parent ? parent.width : 0
                        height: 40

                        color: ListView.isCurrentItem ? Kirigami.Theme.highlightColor : Kirigami.Theme.backgroundColor

                        Label {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                leftMargin: 12
                            }
                            color: Kirigami.Theme.textColor

                            text: tz
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                tzListView.currentIndex = index
                            }
                        }
                    }
                }
            }
        }

        NextButton {
            id: nextButton
            onNextClicked: {
                accepted = true
                timezoneModule.setTimezone(tzListView.currentIndex)
            }

            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: 0
            }
        }

        TimezoneModule {
            id: timezoneModule

            Component.onCompleted: {
                root.moduleName = timezoneModule.moduleName()
                root.moduleIcon = timezoneModule.dir() + "/assets/timezone.svg"
            }

            property var signals: Connections {
                function onSetTimezoneSuccess() {
                    nextButton.next()
                }

                function onSetTimezoneFailed() {
                    nextButton.cancel()
                }

                function onErrorOccurred(err) {
                    console.log(qsTr("TZ ErrorOccurred : %1").arg(err))
                    applicationWindow().showPassiveNotification(err, 2000)
                }
            }
        }
    }
}
