/*
    SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

/**
 * A simple dialog to quickly prompt a user with information,
 * and possibly perform an action.
 *
 * Provides content padding (instead of padding outside of the scroll
 * area). Also has a default preferredWidth, as well as the `subtitle` property.
 *
 * <b>Note:</b> If a `mainItem` is specified, it will replace
 * the subtitle label, and so the respective property will have no effect.
 *
 * @see Dialog
 * @see MenuDialog
 *
 * Example usage:
 *
 * @code{.qml}
 * LingmoUI.PromptDialog {
 *     title: "Reset settings?"
 *     subtitle: "The stored settings for the application will be deleted, with the defaults restored."
 *     standardButtons: LingmoUI.Dialog.Ok | LingmoUI.Dialog.Cancel
 *
 *     onAccepted: console.log("Accepted")
 *     onRejected: console.log("Rejected")
 * }
 * @endcode
 *
 * Text field prompt dialog:
 *
 * @code{.qml}
 * LingmoUI.PromptDialog {
 *     id: textPromptDialog
 *     title: qsTr("New Folder")
 *
 *     standardButtons: LingmoUI.Dialog.NoButton
 *     customFooterActions: [
 *         LingmoUI.Action {
 *             text: qsTr("Create Folder")
 *             icon.name: "dialog-ok"
 *             onTriggered: {
 *                 showPassiveNotification("Created");
 *                 textPromptDialog.close();
 *             }
 *         },
 *         LingmoUI.Action {
 *             text: qsTr("Cancel")
 *             icon.name: "dialog-cancel"
 *             onTriggered: {
 *                 textPromptDialog.close();
 *             }
 *         }
 *     ]
 *
 *     QQC2.TextField {
 *         placeholderText: qsTr("Folder name…")
 *     }
 * }
 * @endcode
 *
 * @inherit Dialog
 */
LingmoUI.Dialog {
    id: root

    default property alias mainItem: mainLayout.data

    enum DialogType {
        Success,
        Warning,
        Error,
        Information,
        None
    }

    /**
     * This property holds the dialogType. It can be either:
     *
     * - `PromptDialog.Success`: For a sucess message
     * - `PromptDialog.Warning`: For a warning message
     * - `PromptDialog.Error`: For an actual error
     * - `PromptDialog.Information`: For an informational message
     * - `PromptDialog.None`: No specific dialog type.
     *
     * By default, the dialogType is `LingmoUI.PromptDialog.None`
     */
    property int dialogType: LingmoUI.PromptDialog.None

    /**
     * The text to use in the dialog's contents.
     */
    property string subtitle

    /**
     * The padding around the content, within the scroll area.
     *
     * Default is `LingmoUI.Units.largeSpacing`.
     */
    property real contentPadding: LingmoUI.Units.largeSpacing

    /**
     * The top padding of the content, within the scroll area.
     */
    property real contentTopPadding: contentPadding

    /**
     * The bottom padding of the content, within the scroll area.
     */
    property real contentBottomPadding: footer.padding === 0 ? contentPadding : 0 // add bottom padding if there is no footer

    /**
     * The left padding of the content, within the scroll area.
     */
    property real contentLeftPadding: contentPadding

    /**
     * The right padding of the content, within the scroll area.
     */
    property real contentRightPadding: contentPadding

    /**
     * This property holds the icon name used by the PromptDialog.
     */
    property string iconName: switch (dialogType) {
    case LingmoUI.PromptDialog.Success:
        return "data-success";
    case LingmoUI.PromptDialog.Warning:
        return "data-warning";
    case LingmoUI.PromptDialog.Error:
        return "data-error";
    case LingmoUI.PromptDialog.Information:
        return "data-information";
    default:
        return "";
    }

    padding: 0
    implicitWidth: Math.min(preferredWidth, maximumWidth) + leftPadding + rightPadding
    preferredWidth: LingmoUI.Units.gridUnit * 18

    header: null

    LingmoUI.Padding {
        id: wrapper

        topPadding: root.contentTopPadding
        leftPadding: root.contentLeftPadding
        rightPadding: root.contentRightPadding
        bottomPadding: root.contentBottomPadding

        contentItem: RowLayout {
            spacing: LingmoUI.Units.largeSpacing

            LingmoUI.Icon {
                source: root.iconName
                visible: root.iconName.length > 0

                Layout.preferredWidth: LingmoUI.Units.iconSizes.huge
                Layout.preferredHeight: LingmoUI.Units.iconSizes.huge
                Layout.alignment: Qt.AlignTop
            }

            ColumnLayout {
                id: mainLayout

                spacing: LingmoUI.Units.smallSpacing

                Layout.fillWidth: true

                ColumnLayout {
                    spacing: 0

                    LingmoUI.Heading {
                        text: root.title
                        visible: root.title.length > 0
                        elide: QQC2.Label.ElideRight
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    LingmoUI.SelectableLabel {
                        text: root.subtitle
                        wrapMode: TextEdit.Wrap
                        visible: text.length > 0
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
