/*
 * SPDX-FileCopyrightText: 2019 Red Hat Inc
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 * SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>
 */

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.platform
import org.kde.kirigami as Kirigami
import org.kde.plasma.workspace.dialogs as PWD

import org.kde.xdgdesktopportal

PWD.SystemDialog {
    id: root

    iconName: "applications-all"

    property bool remember: false
    onRememberChanged: AppChooserData.remember = remember

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.CheckBox {
            Layout.fillWidth: true
            visible: AppChooserData.mimeName !== ""
            text: i18nc("@option:check %1 is description of a file type, like 'PNG image'", "Always open %1 files with the chosen app", AppChooserData.mimeDesc)
            checked: root.remember
            onToggled: {
                root.remember = checked;
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.ComboBox {
                id: searchField
                property bool ready: false
                property string text: editText

                implicitWidth: Kirigami.Units.gridUnit * 20
                Layout.fillWidth: true
                focus: true
                editable: true

                Keys.onDownPressed: {
                    grid.forceActiveFocus();
                    grid.currentIndex = 0;
                }
                model: AppChooserData.history
                onModelChanged: {
                    editText = ""
                    ready = true
                }
                onEditTextChanged: {
                    if (!ready) {
                        return
                    }
                    AppModel.filter = editText;
                    if (editText.length > 0 && grid.count === 1) {
                        grid.currentIndex = 0;
                    }
                }
                onAccepted: grid.currentItem.activate();
            }

            QQC2.Button {
                icon.name: "view-more-symbolic"
                text: i18n("Show All Installed Applications")

                checkable: true
                checked: !AppModel.showOnlyPreferredApps
                visible: AppModel.sourceModel.hasPreferredApps
                onVisibleChanged: AppModel.showOnlyPreferredApps = visible

                onToggled: AppModel.showOnlyPreferredApps = !AppModel.showOnlyPreferredApps
            }

            QQC2.Button {
                visible: AppChooserData.shellAccess
                icon.name: "document-open-symbolic"
                text: i18nc("@action:button", "Choose Other…")
                onClicked: {
                    const text = AppChooserData.openFileDialog()
                    if (text !== "") {
                        searchField.editText = text
                    }
                }
                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: Kirigami.Settings.tabletMode ? pressed : hovered
                QQC2.ToolTip.delay: Kirigami.Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : Kirigami.Units.toolTipDelay
            }
        }

        QQC2.ScrollView {
            id: scrollView

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: grid.cellHeight * 3

            Component.onCompleted: {
                if (background) {
                    background.visible = true;
                }
            }

            GridView {
                id: grid

                readonly property int gridDelegateSize: Kirigami.Units.iconSizes.huge + (Kirigami.Units.gridUnit * 4)

                clip: true

                Keys.onReturnPressed: currentItem.activate();
                Keys.onEnterPressed: currentItem.activate();

                currentIndex: -1 // Don't pre-select anything as that doesn't make sense here

                cellWidth: {
                    const columns = Math.max(Math.floor(scrollView.availableWidth / gridDelegateSize), 2);
                    return Math.floor(scrollView.availableWidth / columns) - 1;
                }
                cellHeight: gridDelegateSize

                model: AppModel
                delegate: Item {
                    id: delegate

                    required property int index
                    required property var model

                    height: grid.cellHeight
                    width: grid.cellWidth

                    function activate() {
                        AppChooserData.applicationSelected(model.applicationDesktopFile, root.remember)
                    }

                    HoverHandler {
                        id: hoverhandler
                    }

                    TapHandler {
                        id: taphandler
                        onTapped: delegate.activate()
                    }

                    Rectangle {
                        anchors.fill: parent
                        visible: hoverhandler.hovered || delegate.GridView.isCurrentItem
                        border.color: Kirigami.Theme.highlightColor
                        border.width: 1
                        color: taphandler.pressed ? Kirigami.Theme.highlightColor : Qt.alpha(Kirigami.Theme.highlightColor, 0.3)
                        radius: Kirigami.Units.smallSpacing
                    }

                    ColumnLayout {
                        anchors {
                            top: parent.top
                            left: parent.left
                            right: parent.right
                            margins: Kirigami.Units.largeSpacing
                        }
                        spacing: 0 // Items add their own as needed here

                        Kirigami.Icon {
                            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                            Layout.bottomMargin: Kirigami.Units.largeSpacing
                            Layout.alignment: Qt.AlignHCenter
                            source: delegate.model.applicationIcon
                            smooth: true
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignTop
                            horizontalAlignment: Text.AlignHCenter
                            text: delegate.model.applicationName
                            font.bold: delegate.model.applicationDesktopFile === AppChooserData.defaultApp
                            elide: Text.ElideRight
                            maximumLineCount: 2
                            wrapMode: Text.WordWrap
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignTop
                            horizontalAlignment: Text.AlignHCenter
                            visible: delegate.model.applicationDesktopFile === AppChooserData.defaultApp || delegate.model.applicationDesktopFile === AppChooserData.lastUsedApp
                            font.bold: true
                            opacity: 0.7
                            text: delegate.model.applicationDesktopFile === AppChooserData.defaultApp
                                ? i18n("Default app for this file type")
                                : i18nc("@info:whatsthis", "Last used app for this file type")
                            elide: Text.ElideRight
                            maximumLineCount: 2
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                Loader {
                    id: placeholderLoader

                    anchors.centerIn: parent
                    width: parent.width - Kirigami.Units.gridUnit * 4

                    active: grid.count === 0
                    sourceComponent: Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent

                        icon.name: "edit-none"
                        text: searchField.editText.length > 0 ? i18n("No matches") : xi18nc("@info", "No installed applications can open <filename>%1</filename>", AppChooserData.fileName)

                        helpfulAction: QQC2.Action {
                            icon.name: "plasmadiscover"
                            text: i18nc("Find some more apps that can open this content using the Discover app", "Find More in Discover")
                            onTriggered: AppChooserData.openDiscover()
                        }
                    }
                }
            }
        }

        ColumnLayout {
            visible: AppChooserData.shellAccess

            RowLayout {
                id: rowLayout
                spacing: Kirigami.Units.largeSpacing
                property bool expanded: false
                onExpandedChanged: containerItem.visibleHeight = expanded ? -1 : 0

                Kirigami.Heading {
                    Layout.fillWidth: rowLayout.children.length === 1
                    Layout.alignment: Qt.AlignVCenter

                    opacity: 0.7
                    level: 5
                    type: Kirigami.Heading.Primary
                    text: i18nc("@title:group", "Terminal Options")
                    elide: Text.ElideRight

                    // we override the Primary type's font weight (DemiBold) for Bold for contrast with small text
                    font.weight: Font.Bold
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            QQC2.CheckBox {
                id: openInTerminal
                onCheckedChanged: AppChooserData.openInTerminal = checked
                text: i18nc("@option:check", "Run in terminal")
            }

            QQC2.CheckBox {
                // NOTE: this only ever works for konsole and xterm as per KTerminalLauncherJob. Trouble is this
                // information is not exposed through API, so we have no way of hiding this when not supported.
                Layout.leftMargin: Kirigami.Units.gridUnit
                enabled: openInTerminal.checked
                onCheckedChanged: AppChooserData.lingerTerminal = checked
                text: i18nc("@option:check", "Do not close when command exits")
            }
        }

        RowLayout {
            visible: discoverMoreEdit.visible
            Kirigami.Heading {
                Layout.fillWidth: rowLayout.children.length === 1
                Layout.alignment: Qt.AlignVCenter

                opacity: 0.7
                level: 5
                type: Kirigami.Heading.Primary
                text: i18nc("@title:group", "Discover More Applications")
                elide: Text.ElideRight

                // we override the Primary type's font weight (DemiBold) for Bold for contrast with small text
                font.weight: Font.Bold
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }
        }

        // Using a TextEdit here instead of a Label because it can know when any
        // links are hovered, which is needed for us to be able to use the correct
        // cursor shape for it.

        TextEdit {
            id: discoverMoreEdit
            visible: !placeholderLoader.active && StandardPaths.findExecutable("plasma-discover").toString() !== ""
            Layout.fillWidth: true
            text: xi18nc("@info", "Don't see the right app? Find more in <link>Discover</link>.")
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            readOnly: true
            color: Kirigami.Theme.textColor
            selectedTextColor: Kirigami.Theme.highlightedTextColor
            selectionColor: Kirigami.Theme.highlightColor

            onLinkActivated: {
                AppChooserData.openDiscover()
            }

            HoverHandler {
                acceptedButtons: Qt.NoButton
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }
    }
}

