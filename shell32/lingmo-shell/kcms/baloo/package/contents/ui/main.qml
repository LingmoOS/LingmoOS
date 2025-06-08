/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.11 as QQC2
import QtQuick.Dialogs 1.2 as QtDialogs
import org.kde.baloo.experimental 0.1 as Baloo
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.3 as KCM

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 42
    implicitHeight: Kirigami.Units.gridUnit * 25

    KCM.ConfigModule.quickHelp: i18n("This module lets you configure the file indexer and search functionality.")

    Connections {
        target: kcm
        function onIndexingSettingsChanged() {
            needToRebootAfterChangingIndexingSettingsMessage.visible = true;
        }
    }
    Baloo.Monitor {
        id: monitor

        readonly property bool currentlyIndexing: switch(state) {
            case Baloo.Global.FirstRun:
            case Baloo.Global.NewFiles:
            case Baloo.Global.ModifiedFiles:
            case Baloo.Global.XAttrFiles:
            case Baloo.Global.ContentIndexing:
            case Baloo.Global.UnindexedFileCheck:
            case Baloo.Global.StaleIndexEntriesClean:
                return true;
            default:
                return false;
        }

        readonly property int completionPercentage: Math.floor(filesIndexed / totalFiles * 100)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: !fileSearchEnabled.checked && kcm.needsSave
            type: Kirigami.MessageType.Warning
            showCloseButton: true
            text: i18n("This will disable file searching in LingmoRunnerand launcher menus, and remove extended metadata display from all KDE applications.");
        }

        Kirigami.InlineMessage {
            id: indexingDisabledWarning
            Layout.fillWidth: true
            visible: !kcm.balooSettings.indexingEnabled && !kcm.needsSave && kcm.rawIndexFileSize() > 0
            type: Kirigami.MessageType.Warning
            showCloseButton: true
            text: i18n("Do you want to delete the saved index data? %1 of space will be freed, but if indexing is re-enabled later, the entire index will have to be re-created from scratch. This may take some time, depending on how many files you have.", kcm.prettyIndexFileSize());
            actions: Kirigami.Action {
                text: i18n("Delete Index Data")
                icon.name: "edit-delete"
                onTriggered: {
                    kcm.deleteIndex();
                    indexingDisabledWarning.visible = false;
                }
            }
        }

        // TODO: remove this message (and the associated Connections object here
        // plus the signal and function on the cpp side) once
        // https://bugs.kde.org/show_bug.cgi?id=462009 is fixed because indexing
        // workers either respect changed settings immediately, or else get killed
        // and re-launched in the background
        Kirigami.InlineMessage {
            id: needToRebootAfterChangingIndexingSettingsMessage
            Layout.fillWidth: true
            visible: false
            type: Kirigami.MessageType.Information
            showCloseButton: true
            text: i18n("The system must be restarted before these changes will take effect.");
            actions: [
                Kirigami.Action {
                    icon.name: "system-reboot"
                    text: i18nc("@action:button", "Restart")
                    onTriggered: kcm.requestReboot();
                }
            ]
        }

        QQC2.Label {
            text: i18n("File Search helps you quickly locate all your files based on their content.")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }

        QQC2.CheckBox {
            id: fileSearchEnabled
            text: i18n("Enable File Search")
            checked: kcm.balooSettings.indexingEnabled
            onCheckStateChanged: {
                kcm.balooSettings.indexingEnabled = checked
            }

            KCM.SettingStateBinding {
                configObject: kcm.balooSettings
                settingName: "indexingEnabled"
            }
        }

        QQC2.CheckBox {
            id: indexFileContents
            Layout.leftMargin: Kirigami.Units.gridUnit
            text: i18n("Also index file content")
            checked: !kcm.balooSettings.onlyBasicIndexing
            onCheckStateChanged: kcm.balooSettings.onlyBasicIndexing = !checked

            KCM.SettingStateBinding {
                configObject: kcm.balooSettings
                settingName: "onlyBasicIndexing"
                extraEnabledConditions: fileSearchEnabled.checked
            }
        }

        QQC2.CheckBox {
            id: indexHiddenFolders
            Layout.leftMargin: Kirigami.Units.gridUnit
            text: i18n("Index hidden files and folders")
            checked: kcm.balooSettings.indexHiddenFolders
            onCheckStateChanged: kcm.balooSettings.indexHiddenFolders = checked

            KCM.SettingStateBinding {
                configObject: kcm.balooSettings
                settingName: "indexHiddenFolders"
                extraEnabledConditions: fileSearchEnabled.checked
            }
        }

        Item {
            Layout.preferredHeight: Kirigami.Units.gridUnit
        }

        // Current status, if indexing
        ColumnLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.gridUnit

            visible: fileSearchEnabled.checked

            RowLayout {
                QQC2.Label {
                    text: i18n("Status: %1, %2\% complete", monitor.stateString, monitor.completionPercentage)
                }

                QQC2.Button {
                    icon.name: monitor.state !== Baloo.Global.Suspended ? "media-playback-pause" : "media-playback-start"
                    text: monitor.state !== Baloo.Global.Suspended ? i18n("Pause Indexer") : i18n("Resume Indexer")
                    onClicked: monitor.toggleSuspendState()
                }
            }

            QQC2.Label {
                id: filePath

                Layout.fillWidth: true

                visible: text.length > 0

                elide: Text.ElideMiddle
                text: monitor.currentlyIndexing && monitor.completionPercentage !== 100 && monitor.filePath ? i18n("Currently indexing: %1", monitor.filePath) : ""
            }
        }

        QQC2.Label {
            text: i18n("Folder specific configuration:")
        }

        QQC2.ScrollView {
            id: bgObject
            Component.onCompleted: bgObject.background.visible = true
            Layout.fillWidth: true
            Layout.fillHeight: true

            // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
            QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

            enabled: fileSearchEnabled.checked

            ListView {
                id: directoryConfigList
                clip: true
                currentIndex: -1

                model: kcm.filteredModel
                delegate: directoryConfigDelegate
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: Kirigami.Units.smallSpacing

            QQC2.Button {
                enabled: fileSearchEnabled.checked
                text: i18n("Start indexing a folder…")
                icon.name: "list-add"

                onClicked: {
                    fileDialogLoader.included = true
                    fileDialogLoader.active = true
                }
            }

            QQC2.Button {
                enabled: fileSearchEnabled.checked
                text: i18n("Stop indexing a folder…")
                icon.name: "list-remove"

                onClicked: {
                    fileDialogLoader.included = false
                    fileDialogLoader.active = true
                }
            }
        }
    }

    Component {
        id: directoryConfigDelegate
        Kirigami.AbstractListItem {
            id: listItem

            // Store this as a property so we can access it within the combobox,
            // which also has a `model` property
            property var indexingModel: model

            // There's no need for a list item to ever be selected
            highlighted: false
            hoverEnabled: false

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                // The folder's icon
                Kirigami.Icon {
                    source: indexingModel.decoration

                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredWidth: Layout.preferredHeight
                }

                // The folder's path
                QQC2.Label {
                    text: indexingModel.folder
                    elide: Text.ElideRight

                    Layout.fillWidth: true
                }

                // What kind of indexing to do for the folder
                QQC2.ComboBox {
                    id: indexingOptionsCombobox

                    property bool indexingDisabled: !indexingModel.enableIndex
                    property bool fullContentIndexing: indexingModel.enableIndex

                    model: [
                        i18n("Not indexed"),
                        i18n("Indexed"),
                    ]

                    // Intentionally not a simple ternary to facilitate adding
                    // more conditions in the future
                    currentIndex: {
                        if (indexingDisabled) return 0
                        if (fullContentIndexing) return 1
                    }

                    onActivated: {
                        // New value is "Not indexed"
                        if (indexingOptionsCombobox.currentIndex === 0 && fullContentIndexing) {
                            indexingModel.enableIndex = false
                        // New value is "Full content indexing"
                        } else if (indexingOptionsCombobox.currentIndex === 1 && indexingDisabled) {
                            indexingModel.enableIndex = true
                        }
                    }
                }

                // Delete button to remove this folder entry
                QQC2.Button {
                    enabled: model.deletable

                    icon.name: "edit-delete"

                    onClicked: kcm.filteredModel.removeFolder(index)

                    QQC2.ToolTip {
                        text: i18n("Delete entry")
                    }
                }
            }
        }
    }

    Loader {
        id: fileDialogLoader

        property bool included: false

        active: false

        sourceComponent: QtDialogs.FileDialog {
            title: fileDialogLoader.included ? i18n("Select a folder to include") : i18n("Select a folder to exclude")
            folder: shortcuts.home
            selectFolder: true

            onAccepted: {
                kcm.filteredModel.addFolder(fileUrls[0], fileDialogLoader.included)
                fileDialogLoader.active = false
            }

            onRejected: {
                fileDialogLoader.active = false
            }

            Component.onCompleted: open()
        }
    }
}
