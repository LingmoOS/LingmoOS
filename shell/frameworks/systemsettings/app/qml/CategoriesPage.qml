/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI
import org.kde.systemsettings

LingmoUI.ScrollablePage {
    id: mainColumn
    readonly property bool searchMode: searchField.text.length > 0

    LingmoUI.Theme.colorSet: LingmoUI.Theme.View
    LingmoUI.Theme.inherit: false

    header: LingmoUI.AbstractApplicationHeader {
        id: pageHeader

        leftPadding: LingmoUI.Units.smallSpacing
        rightPadding: LingmoUI.Units.smallSpacing

        implicitHeight: topPadding + sizeHelper.implicitHeight + bottomPadding

        // Not visible; just to get its size so we can match this custom header
        // with the height of a standard header
        LingmoUI.Heading {
            id: sizeHelper
            // otherwise it gets parented to the content item which we don't want
            parent: pageHeader
            text: "Placeholder"
            visible: false
            textFormat: Text.PlainText
        }

        contentItem: RowLayout {
            id: rowLayout
            // FIXME: left and right anchors shouldn't be needed here, but if
            // they're removed, the layout doesn't span the full width
            anchors.fill: parent
            spacing: Math.round(LingmoUI.Units.smallSpacing/2) // Match margins

            Keys.onDownPressed: event => {
                categoryView.currentIndex = 0;
                event.accepted = false; // Pass to KeyNavigation.down
            }

            LingmoUI.SearchField {
                id: searchField

                focus: !LingmoUI.InputMethod.willShowOnActive
                Layout.fillWidth: true
                onTextChanged: {
                    systemsettings.searchModel.filterRegExp = text;
                }

                KeyNavigation.right: hamburgerMenuButton

                KeyNavigation.down: categoryView

                KeyNavigation.backtab: KeyNavigation.left
                KeyNavigation.tab: KeyNavigation.right

                Keys.onDownPressed: event => rowLayout.Keys.downPressed(event)
            }

            HamburgerMenuButton {
                id: hamburgerMenuButton

                KeyNavigation.left: searchField

                KeyNavigation.down: categoryView

                KeyNavigation.backtab: KeyNavigation.left
                KeyNavigation.tab: KeyNavigation.down

                Keys.onDownPressed: event => rowLayout.Keys.downPressed(event)
            }
        }
    }

    // We don't want it to go into default property and be reparented to a
    // ScrollView, otherwise after clearing search term the scrollbar would
    // appear and shift the fading out placeholder to the left.
    property Item __placeholder: Loader {
        parent: mainColumn
        anchors.centerIn: parent
        width: parent.width - (LingmoUI.Units.gridUnit * 4)
        opacity: categoryView.count == 0 ? 1 : 0
        active: opacity > 0
        visible: active
        Behavior on opacity {
            NumberAnimation {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        sourceComponent: LingmoUI.PlaceholderMessage {
            width: parent.width
            icon.name: "edit-none"
            text: i18nc("A search yielded no results", "No items matching your search")
        }
    }

    Binding {
        target: categoryView
        property: "currentIndex"
        value: mainColumn.searchMode
            ? systemsettings.activeSearchRow
            : systemsettings.activeCategoryRow
    }

    ListView {
        id: categoryView

        model: mainColumn.searchMode ? systemsettings.searchModel : systemsettings.categoryModel

        activeFocusOnTab: true
        keyNavigationWraps: true
        Accessible.role: Accessible.List

        KeyNavigation.up: searchField
        KeyNavigation.backtab: hamburgerMenuButton
        Keys.onUpPressed: event => {
            if (categoryView.currentIndex === 0) {
                categoryView.currentIndex = -1;
            }
            event.accepted = false; // Pass to KeyNavigation.up
        }
        Keys.onTabPressed: {
            systemsettings.focusNext();
        }

        section {
            property: "categoryDisplayRole"
            delegate: LingmoUI.ListSectionHeader {

                required property string section

                width: ListView.view.width
                label: section
            }
        }

        delegate: CategoryItem {
            id: delegate

            required property int index
            required property var model
            required property bool isCategory
            required property int depth
            required property bool isKCM
            required property string iconName

            text: model.display
            icon.name: iconName

            showArrow: {
                if (!isCategory) {
                    return false;
                }
                const modelIndex = delegate.ListView.view.model.index(index, 0)
                return delegate.ListView.view.model.rowCount(modelIndex) > 1
            }
            // Only indent subcategory icons in the search view
            leadingPadding: (depth > 1 && searchField.text.length > 0) ? (( depth - 1 ) * LingmoUI.Units.iconSizes.smallMedium) + LingmoUI.Units.largeSpacing : 0

            hoverEnabled: !isCategory || !mainColumn.searchMode
            enabled: !isCategory || !mainColumn.searchMode

            highlighted: ListView.isCurrentItem

            onClicked: {

                if (isKCM || mainColumn.searchMode || systemsettings.activeCategoryRow !== index) {
                    systemsettings.loadModule(categoryView.model.index(index, 0));
                }
                if (!mainColumn.searchMode && root.pageStack.depth > 1) {
                    root.pageStack.currentIndex = 1;
                }
            }
            onFocusChanged: {
                if (isCategory && mainColumn.searchMode) {
                    return;
                }
                if (focus) {
                    categoryView.positionViewAtIndex(index, ListView.Contain);
                }
            }
            Keys.onEnterPressed: clicked();
            Keys.onReturnPressed: clicked();

            Keys.onLeftPressed: {
                if (LayoutMirroring.enabled) {
                    clicked();
                }
            }
            Keys.onRightPressed: {
                if (!LayoutMirroring.enabled) {
                    clicked();
                }
            }
        }
    }
}
