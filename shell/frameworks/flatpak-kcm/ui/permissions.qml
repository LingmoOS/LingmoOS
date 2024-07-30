/**
 * SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCM
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.kcm.flatpakpermissions

KCM.ScrollViewKCM {
    id: root

    required property FlatpakReference ref

    title: i18n("Permissions")
    implicitWidth: LingmoUI.Units.gridUnit * 15
    framedView: false

    LingmoUI.PlaceholderMessage {
        text: i18n("Select an application from the list to view its permissions here")
        width: parent.width - (LingmoUI.Units.largeSpacing * 4)
        anchors.centerIn: parent
        visible: ref === null
    }

    LingmoUI.Separator {
        anchors.left: parent.left
        height: parent.height
    }

    Component {
        id: addEnvironmentVariableDialogComponent

        AddEnvironmentVariableDialog {
            parent: root.LingmoUI.ColumnView.view
            model: permsModel

            onClosed: destroy()
        }
    }

    Component {
        id: textPromptDialogComponent

        TextPromptDialog {
            parent: root.LingmoUI.ColumnView.view
            model: permsModel

            onClosed: destroy()
        }
    }

    // Having it inside a component helps to separate layouts and workarounds for nullable property accesses.
    Component {
        id: headerComponent

        RowLayout {
            id: header

            readonly property url icon: root.ref.iconSource
            readonly property string title: root.ref.displayName
            readonly property string subtitle: root.ref.version

            spacing: 0

            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin

            LingmoUI.Icon {
                // Fallback doesn't kick in when source is an empty string/url
                source: header.icon.toString() !== "" ? header.icon : "application-vnd.flatpak.ref"

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: LingmoUI.Units.iconSizes.large
                Layout.preferredHeight: LingmoUI.Units.iconSizes.large

                // RowLayout is incapable of paddings, so use margins on both child items instead.
                Layout.margins: LingmoUI.Units.largeSpacing
            }
            ColumnLayout {
                spacing: LingmoUI.Units.smallSpacing
                Layout.fillWidth: true

                Layout.margins: LingmoUI.Units.largeSpacing
                Layout.leftMargin: 0

                LingmoUI.Heading {
                    text: header.title
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                LingmoUI.Heading {
                    text: header.subtitle
                    type: LingmoUI.Heading.Secondary
                    level: 3
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }
        }
    }

    view: ListView {
        model: FlatpakPermissionModel {
            id: permsModel
            reference: root.ref
        }

        currentIndex: -1
        // Mitigate ListView's poor layouting stills.
        // Without it, section delegates may shift down and overlap.
        reuseItems: false
        cacheBuffer: 10000

        // Ref is assumed to remain constant for the lifetime of this page. If
        // it's not null, then it would remain so, and no further checks are
        // needed inside the component.
        header: root.ref === null ? null : headerComponent
        headerPositioning: ListView.InlineHeader

        section.property: "section"
        section.criteria: ViewSection.FullString
        section.delegate: LingmoUI.ListSectionHeader {
            id: sectionDelegate

            required property string section

            /**
             * Sorry about this mess. ListView automatically converts data of
             * section role to string, so we parse it back into enum.
             */
            readonly property /*FlatpakPermissionsSectionType::Type*/ int sectionType: parseInt(section)

            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
            label: permsModel.sectionHeaderForSectionType(sectionType)
            QQC2.ToolButton {
                text: permsModel.showAdvanced ? i18n("Hide advanced permissions") : i18n("Show advanced permissions")
                display: QQC2.AbstractButton.IconOnly
                icon.name: permsModel.showAdvanced ? "collapse" : "expand"
                visible: sectionDelegate.sectionType === FlatpakPermissionsSectionType.Advanced
                onClicked: permsModel.showAdvanced = !permsModel.showAdvanced
                Layout.alignment: Qt.AlignRight

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: LingmoUI.Settings.tabletMode ? pressed : hovered
                QQC2.ToolTip.delay: LingmoUI.Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : LingmoUI.Units.toolTipDelay
            }
            QQC2.ToolButton {
                text: i18n("Add New")
                icon.name: "bqm-add"
                visible: [
                    FlatpakPermissionsSectionType.Filesystems,
                    FlatpakPermissionsSectionType.SessionBus,
                    FlatpakPermissionsSectionType.SystemBus,
                    FlatpakPermissionsSectionType.Environment,
                ].includes(sectionDelegate.sectionType)
                onClicked: {
                    if (sectionDelegate.sectionType === FlatpakPermissionsSectionType.Environment) {
                        const dialog = addEnvironmentVariableDialogComponent.createObject(root, {
                            model: permsModel,
                        });
                        dialog.open();
                    } else {
                        const dialog = textPromptDialogComponent.createObject(root, {
                            model: permsModel,
                            sectionType: sectionDelegate.sectionType,
                        });
                        dialog.open();
                    }
                }
                Layout.alignment: Qt.AlignRight

                QQC2.ToolTip.text: permsModel.sectionAddButtonToolTipTextForSectionType(sectionDelegate.sectionType)
                QQC2.ToolTip.visible: LingmoUI.Settings.tabletMode ? pressed : hovered
                QQC2.ToolTip.delay: LingmoUI.Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : LingmoUI.Units.toolTipDelay
            }
        }

        // Can't use a CheckDelegate or one of its subclasses since we need the checkbox
        // to highlight when it's in a non-default state and none of the pre-made delegates
        // can do that
        delegate: QQC2.ItemDelegate {
            id: permItem

            required property var model
            required property int index

            function toggleAndUncheck() {
                if (checkable) {
                    permsModel.togglePermissionAtRow(index);
                }
                ListView.view.currentIndex = -1;
            }

            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin

            // Default-provided custom entries are not meant to be unchecked:
            // it is a meaningless undefined operation.
            checkable: model.canBeDisabled

            visible: model.isNotDummy

            onClicked: toggleAndUncheck()

            contentItem: RowLayout {
                spacing: LingmoUI.Units.smallSpacing

                QQC2.CheckBox {
                    id: checkBox
                    enabled: permItem.checkable
                    checked: permItem.model.isEffectiveEnabled

                    onToggled: permItem.toggleAndUncheck()

                    KCM.SettingHighlighter {
                        highlight: permItem.model.isEffectiveEnabled !== permItem.model.isDefaultEnabled
                    }
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    text: model.description
                    elide: Text.ElideRight
                }

                QQC2.ComboBox {
                    visible: [
                        FlatpakPermissionsSectionType.Filesystems,
                        FlatpakPermissionsSectionType.SessionBus,
                        FlatpakPermissionsSectionType.SystemBus,
                    ].includes(permItem.model.section)

                    enabled: checkBox.checked

                    model: permItem.model.valuesModel
                    textRole: "display"
                    valueRole: "value"

                    KCM.SettingHighlighter {
                        highlight: permItem.model.effectiveValue !== permItem.model.defaultValue
                    }

                    onActivated: index => {
                        // Assuming this is only called for appropriate visible entries.
                        permsModel.setPermissionValueAtRow(permItem.index, currentValue);
                    }

                    Component.onCompleted: {
                        // Still need to check section type, as this is called for every entry.
                        if (permItem.model.isNotDummy && [
                                FlatpakPermissionsSectionType.Filesystems,
                                FlatpakPermissionsSectionType.SessionBus,
                                FlatpakPermissionsSectionType.SystemBus,
                            ].includes(permItem.model.section)) {
                            currentIndex = Qt.binding(() => indexOfValue(permItem.model.effectiveValue));
                        }
                    }
                }

                QQC2.TextField {
                    visible: permItem.model.isNotDummy && permItem.model.section === FlatpakPermissionsSectionType.Environment
                    text: (permItem.model.isNotDummy && permItem.model.section === FlatpakPermissionsSectionType.Environment)
                        ? permItem.model.effectiveValue : ""
                    enabled: checkBox.checked

                    onTextEdited: {
                        permsModel.setPermissionValueAtRow(permItem.index, text);
                    }

                    KCM.SettingHighlighter {
                        highlight: permItem.model.effectiveValue !== permItem.model.defaultValue
                    }
                }
            }
        }
    }
}
