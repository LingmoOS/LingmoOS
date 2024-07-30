/*
    SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtCore
import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 6.3 as QtDialogs
import QtQuick.Controls 2.10 as QtControls
import org.kde.lingmoui 2.10 as LingmoUI
import org.kde.newstuff 1.91 as NewStuff

LingmoUI.Page {
    id: gtkStylePage
    title: i18n("GNOME/GTK Application Style")

    header: LingmoUI.InlineMessage {
        id: infoLabel

        position: LingmoUI.InlineMessage.Position.Header
        showCloseButton: true
        visible: false

        Connections {
            target: kcm.gtkPage
            function onShowErrorMessage(message) {
                infoLabel.type = LingmoUI.MessageType.Error;
                infoLabel.text = message;
                infoLabel.visible = true;
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        LingmoUI.FormLayout {
            wideMode: true

            Row {
                LingmoUI.FormData.label: i18n("GTK theme:")

                Flow {
                    spacing: LingmoUI.Units.smallSpacing

                    QtControls.ComboBox {
                        id: gtkThemeCombo
                        model: kcm.gtkPage.gtkThemesModel
                        currentIndex: model.findThemeIndex(kcm.gtkPage.gtkThemesModel.selectedTheme)
                        onCurrentTextChanged: function() {
                            model.selectedTheme = currentText
                            gtkRemoveButton.enabled = model.selectedThemeRemovable()
                        }
                        onActivated: model.setSelectedThemeDirty()
                        textRole: "theme-name"
                    }

                    QtControls.Button {
                        id: gtkRemoveButton
                        icon.name: "edit-delete"
                        onClicked: gtkThemeCombo.model.removeSelectedTheme()
                    }

                    QtControls.Button {
                        icon.name: "preview"
                        text: i18n("Preview…")
                        onClicked: kcm.gtkPage.showGtkPreview()
                        visible: kcm.gtkPage.gtkPreviewAvailable()
                    }

                }
            }

        }

        Item {
            Layout.fillHeight: true
        }

        LingmoUI.ActionToolBar {
            flat: false
            alignment: Qt.AlignRight
            actions: [
                LingmoUI.Action {
                    text: i18n("Install from File…")
                    icon.name: "document-import"
                    onTriggered: fileDialogLoader.active = true
                },
                NewStuff.Action {
                    text: i18n("Get New GNOME/GTK Application Styles…")
                    configFile: "gtk_themes.knsrc"
                    onEntryEvent: function (entry, event) {
                        if (event == NewStuff.Entry.StatusChangedEvent) {
                            kcm.load();
                        }
                    }
                }
            ]
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: QtDialogs.FileDialog {
            title: i18n("Select GTK Theme Archive")
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
            nameFilters: [ i18n("GTK Theme Archive (*.tar.xz *.tar.gz *.tar.bz2)") ]
            Component.onCompleted: open()
            onAccepted: {
                kcm.gtkPage.installGtkThemeFromFile(selectedFile)
                fileDialogLoader.active = false
            }
            onRejected: {
                fileDialogLoader.active = false
            }
        }
    }
}
