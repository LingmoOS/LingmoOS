/*
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls 2.8 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kquickcontrols 2.0 as KQC2
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.wallpapers.potd 1.0

Kirigami.FormLayout {
    id: root
    twinFormLayouts: parentLayout

    property string cfg_Provider
    property int cfg_FillMode
    property alias cfg_Color: colorButton.color
    property int cfg_UpdateOverMeteredConnection
    property alias formLayout: root

    readonly property size screenSize: Qt.size(Screen.width, Screen.height)

    PotdBackend {
        id: backend
        identifier: cfg_Provider
        arguments: {
            if (identifier === "bing") {
                // Bing supports 1366/1920/UHD resolutions
                const w = screenSize.width * Screen.devicePixelRatio > 1920 ? 3840 : 1920;
                const h = screenSize.height * Screen.devicePixelRatio > 1080 ? 2160 : 1080;
                return [w, h];
            }
            return [];
        }
        updateOverMeteredConnection: cfg_UpdateOverMeteredConnection
    }

    Row {
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Provider:")

        QQC2.ComboBox {
            id: providerComboBox
            model: PotdProviderModel { }
            currentIndex: model.indexOf(cfg_Provider)
            textRole: "display"
            valueRole: "id"
            onCurrentValueChanged: {
                if (currentIndex < 0) {
                    return;
                }
                cfg_Provider = currentValue;
            }
        }

        Kirigami.ContextualHelpButton {
            anchors.verticalCenter: providerComboBox.verticalCenter
            visible: providerComboBox.model.isNSFW(providerComboBox.currentIndex)
            toolTipText: i18ndc("plasma_wallpaper_org.kde.potd", "@info:tooltip", "This wallpaper provider does not filter out images that may be sensitive or objectionable. Use caution if these images will be displayed in public.")
        }
    }

    QQC2.CheckBox {
        id: updateOverMeteredConnectionCheckBox

        checked: cfg_UpdateOverMeteredConnection === 1
        visible: backend.networkInfomationAvailable
        text: i18ndc("plasma_wallpaper_org.kde.potd", "@option:check", "Update when using metered network connection")

        onToggled: {
            cfg_UpdateOverMeteredConnection = checked ? 1 : 0;
        }
    }

    onCfg_FillModeChanged: {
        resizeComboBox.setMethod()
    }

    QQC2.ComboBox {
        id: resizeComboBox
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Positioning:")
        model: [
                    {
                        'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Scaled and Cropped"),
                        'fillMode': Image.PreserveAspectCrop
                    },
                    {
                        'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Scaled"),
                        'fillMode': Image.Stretch
                    },
                    {
                        'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Scaled, Keep Proportions"),
                        'fillMode': Image.PreserveAspectFit
                    },
                    {
                        'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Centered"),
                        'fillMode': Image.Pad
                    },
                    {
                        'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Tiled"),
                        'fillMode': Image.Tile
                    }
                ]

        textRole: "label"
        onActivated: cfg_FillMode = model[currentIndex]["fillMode"]
        Component.onCompleted: setMethod();

        function setMethod() {
            for (var i = 0; i < model.length; i++) {
                if (model[i]["fillMode"] === cfg_FillMode) {
                    resizeComboBox.currentIndex = i;
                    break;
                }
            }
        }
    }

    KQC2.ColorButton {
        id: colorButton
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:chooser", "Background color:")
        dialogTitle: i18ndc("plasma_wallpaper_org.kde.potd", "@title:window", "Select Background Color")
    }

    Kirigami.Separator {
        id: previewSeparator
        Kirigami.FormData.isSection: true
        visible: wallpaperPreview.visible
    }

    WallpaperPreview {
        id: wallpaperPreview
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label", "Today's picture:")
        backgroundColor: cfg_Color
        visible: !!cfg_Provider // provider is not empty
    }

    Item {
        width: wallpaperPreview.implicitWidth
        height: wallpaperPreview.shadowOffset
    }

    Item {
        Kirigami.FormData.isSection: false
    }

    Kirigami.SelectableLabel {
        id: titleLabel
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label", "Title:")
        Layout.fillWidth: true
        Layout.maximumWidth: wallpaperPreview.implicitWidth * 1.5
        visible: wallpaperPreview.visible && backend.title.length > 0
        font.bold: true
        text: backend.title
        Accessible.name: titleLabel.Kirigami.FormData.label
    }

    Item {
        Kirigami.FormData.isSection: false
    }

    Kirigami.SelectableLabel {
        id: authorLabel
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label", "Author:")
        Layout.fillWidth: true
        Layout.maximumWidth: wallpaperPreview.implicitWidth * 1.5
        visible: wallpaperPreview.visible && backend.author.length > 0
        text: backend.author
        Accessible.name: authorLabel.Kirigami.FormData.label
    }

    Kirigami.InlineMessage {
        id: saveMessage

        Kirigami.FormData.isSection: true
        Layout.fillWidth: true

        showCloseButton: true

        actions: [
            Kirigami.Action {
                icon.name: "document-open-folder"
                text: i18ndc("plasma_wallpaper_org.kde.potd", "@action:button", "Open Containing Folder")
                visible: backend.saveStatus === Global.Successful
                onTriggered: Qt.openUrlExternally(backend.savedFolder)

                Accessible.description: i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis for a button", "Open the destination folder where the wallpaper image was saved.")
            }
        ]

        onLinkActivated: Qt.openUrlExternally(backend.savedUrl)

        Connections {
            target: backend

            function onSaveStatusChanged() {
                switch (backend.saveStatus) {
                case Global.Successful:
                    saveMessage.text = backend.saveStatusMessage;
                    saveMessage.type = Kirigami.MessageType.Positive;
                    break;
                case Global.Failed:
                    saveMessage.text = backend.saveStatusMessage;
                    saveMessage.type = Kirigami.MessageType.Error;
                    break;
                default:
                    return;
                }

                saveMessage.visible = true;
            }
        }
    }
}
