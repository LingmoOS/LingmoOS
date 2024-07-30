/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * This component is intended to be used as root item for
 * KCMs with arbitrary content. Unlike SimpleKCM this does NOT
 * provide a scrollable view, The developer will have to manage
 * their own scrollviews.
 * Most of the times SimpleKCM should be used instead
 * @code
 * import QtQuick
 * import QtQuick.Controls as QQC2
 * import QtQuick.Layouts
 * import org.kde.kcmutils as KCMUtils
 *
 * KCMUtils.AbstractKCM {
 *     RowLayout {
 *         QQC2.ScrollView { }
 *         QQC2.ScrollView { }
 *     }
 *     footer: QQC2.ToolBar { }
 * }
 * @endcode
 * @inherits org.kde.kirigami.Page
 * @since 6.0
 */
Kirigami.Page {
    id: root

    readonly property int margins: 6 // Layout_ChildMarginWidth from Breeze

    /**
     * framedView: bool
     * Whether to use this component as the base of a "framed" KCM with an
     * inner scrollview that draws its own frame.
     * Default: true
     */
    property bool framedView: true

    /**
     * extraFooterTopPadding: bool
     * @deprecated unused
     * Default: false
     */
    property bool extraFooterTopPadding: false

    /**
     * headerPaddingEnabled: bool
     * Whether the contents of the header will have automatic padding around it.
     * Should be disabled when using an InlineMessage or custom content item in
     * the header that's intended to touch the window edges.
     * Default: true
     */
    property bool headerPaddingEnabled: true

    /**
     * footerPaddingEnabled: bool
     * Whether the contents of the footer will have automatic padding around it.
     * Should be disabled when using an InlineMessage or custom content item in
     * the footer that's intended to touch the window edges.
     * Default: true
     */
    property bool footerPaddingEnabled: true

    property bool sidebarMode: false

    function __itemVisible(item: Item): bool {
        return item !== null && item.visible && item.implicitHeight > 0;
    }

    function __headerContentVisible(): bool {
        return __itemVisible(headerParent.contentItem);
    }
    function __footerContentVisible(): bool {
        return __itemVisible(footerParent.contentItem);
    }

    // Deliberately not checking for __footerContentVisible because
    // we always want the footer line to be visible when the scrollview
    // doesn't have a frame of its own, because System Settings always
    // adds its own footer for the Apply, Help, and Defaults buttons
    function __headerSeparatorVisible(): bool {
        return !framedView && __headerContentVisible();
    }
    function __footerSeparatorVisible(): bool {
        return !framedView && extraFooterTopPadding;
    }

    title: (typeof kcm !== "undefined") ? kcm.name : ""

    // Make pages fill the whole view by default
    Kirigami.ColumnView.fillWidth: sidebarMode
                                   ? Kirigami.ColumnView.view
                                         && (Kirigami.ColumnView.view.width < Kirigami.Units.gridUnit * 36
                                             || Kirigami.ColumnView.index >= Kirigami.ColumnView.view.count - 1)
                                   : true

    padding: 0
    topPadding: framedView && !__headerContentVisible() ? margins : 0
    leftPadding: undefined
    rightPadding: undefined
    bottomPadding: framedView && !__footerContentVisible() ? margins : 0
    verticalPadding: undefined
    horizontalPadding: framedView ? margins : 0

    header: Column {
        Kirigami.Padding {
            id: headerParent

            anchors {
                left: parent.left
                right: parent.right
            }

            height: root.__headerContentVisible() ? undefined : 0
            padding: root.headerPaddingEnabled ? root.margins : 0
        }

        // When the scrollview isn't drawing its own frame, we need to add a
        // line below the header (when visible) to separate it from the view
        Kirigami.Separator {
            anchors {
                left: parent.left
                right: parent.right
            }

            visible: root.__headerSeparatorVisible()
        }
    }

    // View background, shown when the scrollview isn't drawing its own frame
    Rectangle {
        anchors.fill: parent
        visible: !root.framedView
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
    }

    footer: Column {
        // When the scrollview isn't drawing its own frame, we need to add a
        // line above the footer ourselves to separate it from the view
        Kirigami.Separator {
            anchors {
                left: parent.left
                right: parent.right
            }

            visible: root.__footerSeparatorVisible()
        }

        Kirigami.Padding {
            id: footerParent

            anchors {
                left: parent.left
                right: parent.right
            }

            height: root.__footerContentVisible() ? undefined : 0
            padding: root.footerPaddingEnabled ? root.margins : 0
        }
    }

    function __swapContentIntoContainer(property: string, container: Item): void {
        const content = this[property];
        const rootContainer = container.parent;

        if (content && content !== rootContainer) {
            // Revert the effect of repeated onHeaderChanged invocations
            // during initialization in Page super-type.
            content.anchors.top = undefined;

            this[property] = rootContainer;
            container.contentItem = content;
        }
    }

    function __adoptOverlaySheets(): void {
        // Search overlaysheets in contentItem, parent to root if found
        for (const object of contentItem.data) {
            if (object instanceof Kirigami.OverlaySheet) {
                if (object.parent === null) {
                    object.parent = this;
                }
                data.push(object);
            }
        }
    }

    Component.onCompleted: {
        __swapContentIntoContainer("header", headerParent);
        __swapContentIntoContainer("footer", footerParent);
        __adoptOverlaySheets();
    }
}
