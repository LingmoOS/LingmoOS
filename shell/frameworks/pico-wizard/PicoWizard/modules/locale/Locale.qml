// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.7 as Kirigami

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

                placeholderText: qsTr("Search locales")

                onTextChanged: {
                    localeModule.filterText = text
                }
            }

            Rectangle {
                id: localeContainer
                Layout.preferredWidth: root.width * 0.7
                Layout.fillHeight: true

                radius: 4
                border.width: 2
                border.color: Kirigami.Theme.backgroundColor
                color: Qt.lighter(Kirigami.Theme.backgroundColor, 1.1)

                ScrollIndicator {
                    id: localeScroll
                    width: 12
                    anchors {
                        top: localeContainer.top
                        right: localeContainer.right
                        bottom: localeContainer.bottom
                    }
                }

                ListView {
                    id: localeListView
                    anchors.fill: parent
                    anchors.margins: 8

                    spacing: 4
                    model: localeModule.model
                    clip: true
                    ScrollIndicator.vertical: localeScroll

                    delegate: Rectangle {
                        color: Kirigami.Theme.backgroundColor
                        width: parent ? parent.width : 0
                        height: 40

                        RowLayout {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                leftMargin: 12
                            }

                            CheckBox {
                                checked: selected
                            }

                            Label {
                                color: Kirigami.Theme.textColor
                                text: name
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                var curIndex = localeModule.model.index(index, 0);

                                localeModule.model.setData(
                                    curIndex,
                                    !localeModule.model.data(curIndex, LocaleModel.Roles.SelectedRole),
                                    LocaleModel.Roles.SelectedRole
                                )
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
                localeModule.writeLocaleGenConfig()
            }

            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: 0
            }
        }

        LocaleModule {
            id: localeModule

            Component.onCompleted: {
                root.moduleName = localeModule.moduleName()
                root.moduleIcon = localeModule.dir() + "/assets/locale.svg"
            }

            property var signals: Connections {
                function onLocaleSetupSuccess() {
                    nextButton.next()
                }

                function onLocaleSetupFailed() {
                    nextButton.cancel()
                }

                function onErrorOccurred(err) {
                    console.log(qsTr("Locale Error Occurred : %1").arg(err))
                    applicationWindow().showPassiveNotification(err, 2000)
                }
            }
        }
    }
}
