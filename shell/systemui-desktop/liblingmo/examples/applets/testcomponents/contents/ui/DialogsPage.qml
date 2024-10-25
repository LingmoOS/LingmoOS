/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components as LingmoComponents
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.lingmoui as LingmoUI

// DialogsPage

LingmoComponents.Page {
    id: dialogsPage
    anchors {
        fill: parent
        margins: _s
    }
    Column {
        spacing: _s/2
        anchors.fill: parent
        LingmoUI.Heading {
            width: parent.width
            level: 1
            text: "Dialogs"
        }
        Row {
            height: _h
            spacing: _s
            LingmoComponents.Button {
                id: radio
                checkable: true
                iconSource: "dialog-ok"
                text: "Window"
            }
            Window {
                title: radio.text
                id: qWindow
                visible: radio.checked
                width: childrenRect.width
                height: childrenRect.height
                color: Qt.rgba(0,0,0,0)
                DialogContent {
                    id: dContent
                    onCloseMe: {
                        qWindow.visible = false
                    }
                }
            }

            LingmoComponents.Label {
                text: qWindow.visible ? "shown" : "hidden"
            }
        }
        Row {
            height: _h
            spacing: _s
            LingmoComponents.Button {
                text: "Core.Dialog"
                iconSource: "dialog-ok-apply"
                checkable: true
                //onCheckedChanged: pcDialog.visible = checked
                onCheckedChanged: pcDialog.visible = checked
            }
            LingmoComponents.Label {
                text: pcDialog.visible ? "shown" : "hidden"
            }

            LingmoCore.Dialog {
                id: pcDialog
                //windowFlags: Qt.Popup
                visualParent: dialogsPage
                //mainItem: dContent2
                color: Qt.rgba(0,0,0,0)

                mainItem: DialogContent {
                    id: dContent2
                    onCloseMe: pcDialog.visible = false
                }
            }
        }
        Row {
            height: _h
            spacing: _s
            LingmoComponents.Button {
                text: "Dialog"
                iconSource: "dialog-ok-apply"
                checkable: true
                onCheckedChanged: {
                    if (checked) {
                        pcompDialog.open();
                    } else {
                        pcompDialog.close();
                    }
                }
            }
            LingmoComponents.Label {
                text: pcompDialog.visible ? "shown" : "hidden"
            }

            LingmoComponents.Dialog {
                id: pcompDialog
                //windowFlags: Qt.Popup
                visualParent: root
                content: DialogContent {
                    id: dContent3
                    onCloseMe: pcompDialog.close()
                }
                buttons: LingmoComponents.ButtonRow {
                    LingmoComponents.Button {
                        text: "Close";
                        onClicked: {
                            print("Closing...");
                            pcompDialog.close()
                        }
                    }
                    LingmoComponents.Button {
                        text: "Accept";
                        onClicked: {
                            print("Accepting...");
                            pcompDialog.accept();
                            pcompDialog.close();
                        }
                    }
                }
            }
        }
        Row {
            height: _h
            spacing: _s
            LingmoComponents.Button {
                text: "QueryDialog"
                iconSource: "dialog-ok-apply"
                checkable: true
                onCheckedChanged: {
                    if (checked) {
                        queryDialog.open();
                    } else {
                        queryDialog.close();
                    }
                }
            }
            LingmoComponents.Label {
                text: queryDialog.visible ? "shown" : "hidden"
            }

            LingmoComponents.QueryDialog {
                id: queryDialog
                //windowFlags: Qt.Popup
                visualParent: root
                titleText: "Fruit Inquiry"
                message: "Would you rather have apples or oranges?"
                acceptButtonText: "Apples"
                rejectButtonText: "Oranges"
                onButtonClicked: {
                    print("hey");
                    queryDialog.close();
                }
            }
        }
        LingmoComponents.ButtonRow {
            id: buttonRow
            spacing: _s/2
            LingmoComponents.Button {
                width: _h
                text: "Top"
                onClicked: {
                    locationDialog.location = LingmoCore.Types.TopEdge;
                    locationDialog.visible = !locationDialog.visible
                }
            }
            LingmoComponents.Button {
                text: "Bottom"
                width: _h
                onClicked: {
                    locationDialog.location = LingmoCore.Types.BottomEdge;
                    locationDialog.visible = !locationDialog.visible
                }
            }
            LingmoComponents.Button {
                text: "Left"
                width: _h
                onClicked: {
                    locationDialog.location = LingmoCore.Types.LeftEdge;
                    locationDialog.visible = !locationDialog.visible
                }
            }
            LingmoComponents.Button {
                text: "Right"
                width: _h
                onClicked: {
                    locationDialog.location = LingmoCore.Types.RightEdge;
                    locationDialog.visible = !locationDialog.visible
                }
            }
            LingmoComponents.Button {
                text: "Desktop"
                width: _h
                onClicked: {
                    locationDialog.location = LingmoCore.Types.Desktop;
                    locationDialog.visible = !locationDialog.visible
                }
            }
            LingmoComponents.Button {
                text: "Floating"
                width: _h
                onClicked: {
                    locationDialog.location = LingmoCore.Types.Floating;
                    locationDialog.visible = !locationDialog.visible
                }
            }
            LingmoComponents.Button {
                text: "FullScreen"
                width: _h
                onClicked: {
                    locationDialog.location = LingmoCore.Types.FullScreen;
                    locationDialog.visible = !locationDialog.visible
                }
            }
        }
        LingmoCore.Dialog {
            id: locationDialog
            visualParent: buttonRow
            mainItem: DialogContent {
                id: dContent4
                onCloseMe: locationDialog.visible = false
            }
        }
    }
}

