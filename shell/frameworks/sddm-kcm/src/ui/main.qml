/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Qt5Compat.GraphicalEffects


import org.kde.kcmutils as KCM
import org.kde.kirigami 2.14 as Kirigami
import org.kde.newstuff 1.81 as NewStuff
import org.kde.private.kcms.sddm 1.0

KCM.GridViewKCM {
    id: root
    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Behavior…")
            icon.name: "settings-configure"
            onTriggered: { kcm.push("Advanced.qml") }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Apply Plasma Settings…")
            icon.name: "plasma"
            onTriggered: syncSheet.open()
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Install From File…")
            icon.name: "document-import"
            onTriggered: themeDialog.open()
        },
        NewStuff.Action {
            text: i18nc("@action:button as in, \"get new SDDM themes\"", "Get New…")
            configFile: "sddmtheme.knsrc"
            onEntryEvent: function(entry, event) {
                kcm.themesModel.populate();
            }
        }
    ]

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: Kirigami.InlineMessage {
        id: errorMessage
        position: Kirigami.InlineMessage.Position.Header
        type: Kirigami.MessageType.Error
        showCloseButton: true
        Connections {
            target: kcm

            function onErrorOccured(untranslatedMessage) {
                errorMessage.text = i18n(untranslatedMessage);
                errorMessage.visible = untranslatedMessage.length > 0
            }

            function onSyncSuccessful() {
                syncSheet.close()
            }

            function onResetSyncedDataSuccessful() {
                syncSheet.close()
            }
        }
    }
    KCM.SettingStateBinding {
        configObject: kcm.sddmSettings
        settingName: "Current"
    }
    view.model: kcm.themesModel
    view.currentIndex: kcm.themesModel.currentIndex
    view.delegate: KCM.GridDelegate {
        id: delegate
        text: model.display
        subtitle: model.author
        thumbnailAvailable: true
        thumbnail: Image {
            anchors.fill: parent
            source: model.preview
            sourceSize: Qt.size(delegate.GridView.view.cellWidth * Screen.devicePixelRatio,
                                delegate.GridView.view.cellHeight * Screen.devicePixelRatio)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
        actions: [
            Kirigami.Action {
                icon.name: "documentinfo"
                tooltip: i18nc("@info:tooltip", "View details")
                onTriggered: {
                    detailsDialog.themeName = model.display
                    detailsDialog.previewPath = model.preview
                    detailsDialog.authorName = model.author
                    detailsDialog.description = model.description
                    detailsDialog.license = model.license
                    detailsDialog.email = model.email
                    detailsDialog.website = model.website
                    detailsDialog.version = model.version
                    detailsDialog.open()
                }
            },
            Kirigami.Action {
                icon.name: "games-config-background"
                tooltip: i18nc("@info:tooltip", "Change Background")
                onTriggered: {
                    backgroundSheet.modelIndex = view.model.index(index, 0)
                    backgroundSheet.imagePath = Qt.binding(() => model.currentBackground)
                    backgroundSheet.open()
                }
            },
            Kirigami.Action {
                icon.name: "edit-delete"
                tooltip: i18nc("@info:tooltip", "Delete")
                onTriggered: kcm.removeTheme(view.model.index(index, 0))
                enabled: model.deletable
            }
        ]
        onClicked: kcm.sddmSettings.current = model.id

        onDoubleClicked: {
            kcm.save();
        }
    }
    FileDialog {
        id: themeDialog
        onAccepted: kcm.installTheme(selectedFile)
    }
    DetailsDialog {
        id: detailsDialog
    }
    Kirigami.OverlaySheet {
        id: syncSheet
        title: i18nc("@title:window", "Apply Plasma Settings")
        Kirigami.InlineMessage {
            implicitWidth: Math.max(footer.implicitWidth, Kirigami.Units.gridUnit * 22)
            visible: true
            type: Kirigami.MessageType.Information
            font: Kirigami.Theme.smallFont
            text: i18n("This will make the SDDM login screen reflect your customizations to the following Plasma settings:") +
                xi18nc("@info", "<para><list><item>Color scheme,</item><item>Cursor theme,</item><item>Cursor size,</item><item>Font,</item><item>Font rendering,</item><item>NumLock preference,</item><item>Plasma theme,</item><item>Scaling DPI,</item><item>Screen configuration (Wayland only)</item></list></para>") +
                i18n("Please note that theme files must be installed globally to be reflected on the SDDM login screen.")
        }
        footer: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Item {
                Layout.fillWidth: true
            }
            QQC2.Button {
                text: i18nc("@action:button", "Apply")
                icon.name: "dialog-ok-apply"
                onClicked: kcm.synchronizeSettings()
            }
            QQC2.Button {
                text: i18nc("@action:button", "Reset to Default Settings")
                icon.name: "edit-undo"
                onClicked: kcm.resetSyncronizedSettings()
            }
        }
    }
    Kirigami.OverlaySheet {
        id: backgroundSheet
        property var modelIndex
        property string imagePath
        title: i18nc("@title:window", "Change Background")
        Item {
            implicitWidth: 0.75 * root.width
            implicitHeight: backgroundImage.hasImage ? backgroundImage.implicitHeight : placeHolder.implicitHeight
            Kirigami.PlaceholderMessage  {
                id: placeHolder
                anchors.fill: parent
                visible: !backgroundImage.hasImage
                icon.name: "view-preview"
                text: i18n("No image selected")
            }
            Image {
                id: backgroundImage
                readonly property bool hasImage: status == Image.Ready || status == Image.Loading
                width: parent.width
                source: "file:" + backgroundSheet.imagePath
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit
                smooth: true
                layer.enabled: true
                layer.effect: DropShadow {
                    verticalOffset: 2
                    radius: 10
                    samples: 32
                    cached: true
                    color: Qt.rgba(0, 0, 0, 0.3)
                }
            }
        }
        footer: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Item {
                Layout.fillWidth: true
            }
            QQC2.Button {
                icon.name: "document-open"
                text: i18nc("@action:button", "Load From File…")
                onClicked: imageDialog.open()
            }
            QQC2.Button {
                icon.name: "edit-clear"
                text: i18nc("@action:button", "Clear Image")
                onClicked: {
                    view.model.setData(backgroundSheet.modelIndex, "", ThemesModel.CurrentBackgroundRole)
                    backgroundSheet.close()
                }
            }
            FileDialog {
                id: imageDialog
                onAccepted: {
                    view.model.setData(backgroundSheet.modelIndex, kcm.toLocalFile(selectedFile), ThemesModel.CurrentBackgroundRole)
                    backgroundSheet.close()
                }
            }
        }
    }
}
