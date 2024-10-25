/*
 *  SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmoui as LingmoUI

/**
 * A placeholder message indicating that a list view is empty. The message
 * comprises a label with lightened text, an optional icon above the text, and
 * an optional button below the text which can be used to easily show the user
 * what to do next to add content to the view.
 *
 * The top-level component is a ColumnLayout, so additional components items can
 * simply be added as child items and they will be positioned sanely.
 *
 * Example usage:
 *
 * @code{.qml}
 ** Shows how to use PlaceholderMessage to implement a "this view is empty" message
 * import QtQuick
 * import org.kde.lingmo.extras as LingmoExtras
 *
 * ListView {
 *     id: listView
 *     model: [...]
 *     delegate: [...]
 *
 *     LingmoExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (LingmoUI.Units.gridUnit * 4)
 *
 *         visible: listView.count == 0
 *
 *         text: "There are no items in this list"
 *     }
 * }
 * @endcode
 * @code{.qml}
 ** Shows how to use PlaceholderMessage to implement a "here's how to proceed" message
 * import QtQuick
 * import QtQuick.Controls as QQC2
 * import org.kde.lingmo.extras as LingmoExtras
 *
 * ListView {
 *     id: listView
 *     model: [...]
 *     delegate: [...]
 *
 *     LingmoExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (LingmoUI.Units.gridUnit * 4)
 *
 *         visible: listView.count == 0
 *
 *         text: "Add an item to proceed"
 *
 *         helpfulAction: QQC2.Action {
 *             icon.name: "list-add"
 *             text: "Add item..."
 *             onTriggered: {
 *                 [...]
 *             }
 *         }
 *     }
 *     [...]
 * }
 * @endcode
 * @code{.qml}
 ** Shows how to use PlaceholderMessage to implement a "there was a problem here" message
 * import org.kde.lingmo.components as LingmoComponents3
 * import org.kde.lingmo.extras as LingmoExtras
 *
 * LingmoComponents3.Page {
 *     id: root
 *     readonly property bool networkConnected: [...]
 *
 *     LingmoExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (LingmoUI.Units.gridUnit * 4)
 *
 *         visible: root.networkConnected
 *
 *         iconName: "network-disconnect"
 *         text: "Unable to load content
 *         explanation: "Please try again later"
 *     }
 * }
 * @endcode
 * @code{.qml}
 * import org.kde.lingmo.components as LingmoComponents3
 * import org.kde.lingmo.extras as LingmoExtras
 *
 ** Shows how to use PlaceholderMessage to implement a loading indicator
 * LingmoComponents3.Page {
 *     id: root
 *     readonly property bool loading: [...]
 *     readonly property int completionStatus: [...]
 *
 *     LingmoExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (LingmoUI.Units.gridUnit * 4)
 *
 *         visible: root.loading
 *
 *         iconName: "my-awesome-app-icon"
 *         text: "Loading this awesome app"
 *
 *         LingmoComponents3.ProgressBar {
 *             Layout.preferredWidth: LingmoUI.Units.gridUnit * 20
 *             value: root.completionStatus
 *             from: 0
 *             to: 100
 *         }
 *     }
 * }
 * @endcode
 * @code{.qml}
 * import QtQuick.Controls as QQC2
 * import org.kde.lingmo.components as LingmoComponents3
 * import org.kde.lingmo.extras as LingmoExtras
 *
 ** Shows how to use PlaceholderMessage to implement a "Here's what you do next" button
 * LingmoComponents3.Page {
 *     id: root
 *
 *     LingmoExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (LingmoUI.Units.largeSpacing * 4)
 *
 *         visible: root.loading
 *
 *         helpfulAction: QQC2.Action {
 *             icon.name: "list-add"
 *             text: "Add item..."
 *             onTriggered: {
 *                 [...]
 *             }
 *         }
 *     }
 * }
 * @endcode
 * @since 5.72
 */
ColumnLayout {
    id: root

    enum Type {
        Actionable,
        Informational
    }

    /**
     * The type of the message. This can be:
     *
     * * LingmoExtras.PlaceholderMessage.Type.Actionable: Makes it more attention-getting. Useful when the user is expected to interact with the message.
     * * LingmoExtras.PlaceholderMessage.Type.Informational: Makes it less prominent. Useful when the message in only informational.
     *
     * By default if an helpfulAction is provided this will be of type Actionable otherwise of type Informational.
     * @since 5.94
     */
    property int type: helpfulAction && helpfulAction.enabled ? PlaceholderMessage.Type.Actionable : PlaceholderMessage.Type.Informational

    /**
     * text: string
     * The text to show as a placeholder label
     *
     * Optional; if not defined, the message will have no large text label
     * text. If both text: and explanation: are omitted, the message will have
     * no text and only an icon, action button, and/or other custom content.
     *
     * @since 5.72
     */
    property string text

    /**
     * explanation: string
     * Smaller explanatory text to show below the larger title-style text
     *
     * Useful for providing a user-friendly explanation for how to proceed.
     *
     * Optional; if not defined, the message will have no supplementary
     * explanatory text.
     *
     * @since 5.80
     */
    property string explanation

    /**
     * iconName: string
     * The icon to show above the text label.
     *
     * Optional
     * Falls back to `undefined` if the specified icon is not valid or cannot
     * be loaded.
     *
     * @since 5.72
     * @see Icon::source
     */
    property string iconName

    /**
     * helpfulAction: QtQuickControls2 Action
     * An action that helps the user proceed. Typically used to guide the user
     * to the next step for adding content or items to an empty view.
     *
     * Optional
     *
     * @since 5.72
     */
    property T.Action helpfulAction

    spacing: LingmoUI.Units.gridUnit

    LingmoUI.Icon {
        visible: source !== undefined
        opacity: 0.5

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: Math.round(LingmoUI.Units.iconSizes.huge * 1.5)
        Layout.preferredHeight: Math.round(LingmoUI.Units.iconSizes.huge * 1.5)

        source: root.iconName || null
    }

    LingmoUI.Heading {
        text: root.text
        visible: text.length > 0
        opacity: root.type === PlaceholderMessage.Type.Actionable ? 1 : 0.65

        type: LingmoUI.Heading.Primary

        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter

        wrapMode: Text.WordWrap
    }

    LingmoComponents3.Label {
        text: root.explanation
        visible:  root.explanation !== ""
        opacity: root.type === PlaceholderMessage.Type.Actionable ? 1 : 0.65

        horizontalAlignment: Qt.AlignHCenter
        wrapMode: Text.WordWrap

        Layout.fillWidth: true
    }

    Loader {
        active: root.helpfulAction && root.helpfulAction.enabled
        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: LingmoUI.Units.gridUnit

        sourceComponent: LingmoComponents3.Button {
            action: root.helpfulAction
        }
    }
}
