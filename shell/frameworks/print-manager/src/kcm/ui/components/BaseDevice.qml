/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import ".."

/**
 * This provides the base for a device setup component.
 *
 * Titles, layout, helpers and common items are available
 * while it is intended device specific items be defined in the descendents.
 *
 * Heading info is the first layout item.  After that is an optional URI
 * label/field with optional search.
 *
 * For additional device specific items, use the contentItem.
*/
ColumnLayout {
    id: root
    spacing: Kirigami.Units.largeSpacing*2

    property alias title: heading.title
    property alias subtitle: heading.subtitle
    property alias icon: heading.icon
    property alias helpText: helpLabel.text

    property alias uriText: connSearch.text
    property alias showUri: uriLayout.visible
    property alias showUriSearch: searchButton.visible

    property string scheme: "://"
    property Component contentItem: Item {}

    property alias showAddressExamples: addrLoader.active
    property var addressExamples: []

    signal addressExampleSelected(string address)
    signal uriSearch(string uri)

    Component.onDestruction: {
        kcm.clearRecommendedDrivers()
        kcm.clearRemotePrinters()
    }

    function setError(msg: string) {
        errorMsg.text = msg
        errorMsg.visible = true
    }

    // Validate uri
    function parseUri(uri : string) : bool {
        try {
            const u = new URL(uri)
            return true
        } catch (e) {
            setError(e.message)
            return false
        }
    }

    // Return a URL, return null if it fails
    function getUrl(uri : string) : var {
        try {
            const url = new URL(uri)
            return url
        } catch (error) {
            return null
        }
    }

    BannerWithTimer {
        id: errorMsg
        Layout.fillWidth: true
    }

    Kirigami.IconTitleSubtitle {
        id: heading

        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.15
        subtitleFont.pointSize: Kirigami.Theme.defaultFont.pointSize

        elide: Text.ElideRight

        icon.source: "printer"
        icon.width: Kirigami.Units.iconSizes.huge
        icon.height: Kirigami.Units.iconSizes.huge

        Layout.alignment: Qt.AlignHCenter
        Layout.bottomMargin: Kirigami.Units.largeSpacing
    }

    Kirigami.Heading {
        id: helpLabel
        level: 4
        visible: text.length > 0
        horizontalAlignment: Qt.AlignHCenter
        Layout.fillWidth: true
    }

    // Uri
    RowLayout {
        id: uriLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18nc("@label:textbox", "Address:")
        }

        Kirigami.SearchField {
            id: connSearch
            Layout.fillWidth: true

            focus: true
            delaySearch: true
            autoAccept: false
            placeholderText: i18nc("@info The printer/device address", "Enter device address")

            KeyNavigation.left: root.parent
            KeyNavigation.backtab: KeyNavigation.left
            KeyNavigation.tab: KeyNavigation.right

            onAccepted: searchButton.clicked()
        }

        QQC2.ToolButton {
            id: searchButton
            icon.name: "search-symbolic"
            onClicked: {
                if (connSearch.text.length === 0) {
                    return
                }

                root.uriSearch(connSearch.text)
            }
        }
    }

    // Form content
    Loader {
        sourceComponent: contentItem
    }

    // Address Examples
    Loader {
        id: addrLoader
        active: false

        Layout.fillWidth: true
        Layout.fillHeight: true

        sourceComponent: AddressExamples {
            examples: root.addressExamples
            onSelected: address => {
                            connSearch.text = address
                            root.addressExampleSelected(address)
                        }
        }

    }


}
