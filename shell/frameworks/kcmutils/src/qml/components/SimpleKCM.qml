/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

/**
 * This component is intended to be used as root item for
 * KCMs with arbitrary content. Often a LingmoUI.FormLayout
 * is used as main element.
 * It is possible to specify a header and footer component.
 * @code
 * import org.kde.kcmutils as KCMUtils
 * import org.kde.lingmoui as LingmoUI
 *
 * KCMUtils.SimpleKCM {
 *     LingmoUI.FormLayout {
 *        TextField {
 *           LingmoUI.FormData.label: "Label:"
 *        }
 *        TextField {
 *           LingmoUI.FormData.label: "Label:"
 *        }
 *     }
 *     footer: Item {...}
 * }
 * @endcode
 * @inherits org.kde.lingmoui.ScrollablePage
 */
LingmoUI.ScrollablePage {
    id: root

    readonly property int margins: 6 // Layout_ChildMarginWidth from Ocean

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
     * Default: false
     */
    property bool headerPaddingEnabled: false

    function __itemVisible(item: Item): bool {
        return item !== null && item.visible && item.implicitHeight > 0;
    }

    function __headerContentVisible(): bool {
        return __itemVisible(headerParent.contentItem);
    }

    property bool __flickableOverflows: flickable.contentHeight + flickable.topMargin + flickable.bottomMargin > flickable.height

    // Context properties are not reliable
    title: (typeof kcm !== "undefined") ? kcm.name : ""

    // Make pages fill the whole view by default
    LingmoUI.ColumnView.fillWidth: true

    property bool sidebarMode: false

    topPadding: margins
    leftPadding: margins
    rightPadding: margins
    bottomPadding: margins

    header: Column {
        LingmoUI.Padding {
            id: headerParent

            anchors {
                left: parent.left
                right: parent.right
            }

            height: root.__headerContentVisible() ? undefined : 0
            padding: root.headerPaddingEnabled ? root.margins : 0
        }

        // When the header is visible, we need to add a line below to separate
        // it from the view
        LingmoUI.Separator {
            anchors {
                left: parent.left
                right: parent.right
            }

            visible: root.__headerContentVisible()
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
            if (object instanceof LingmoUI.OverlaySheet) {
                if (object.parent === null) {
                    object.parent = this;
                }
                data.push(object);
            }
        }
    }

    Component.onCompleted: {
        __swapContentIntoContainer("header", headerParent);
        __adoptOverlaySheets();
    }
}
