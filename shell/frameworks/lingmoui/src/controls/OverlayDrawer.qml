/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import "private" as KP
import "templates" as KT

/**
 * Overlay Drawers are used to expose additional UI elements needed for
 * small secondary tasks for which the main UI elements are not needed.
 * For example in Okular Mobile, an Overlay Drawer is used to display
 * thumbnails of all pages within a document along with a search field.
 * This is used for the distinct task of navigating to another page.
 *
 * @inherit org::kde::lingmoui::templates::OverlayDrawer
 */
KT.OverlayDrawer {
    id: root

//BEGIN Properties
    focus: false
    modal: true
    drawerOpen: !modal
    closePolicy: modal ? T.Popup.CloseOnEscape | T.Popup.CloseOnReleaseOutside : T.Popup.NoAutoClose
    handleVisible: interactive && (modal || !drawerOpen) && (typeof(applicationWindow)===typeof(Function) && applicationWindow() ? applicationWindow().controlsVisible : true)

    // FIXME: set to false when it does not lead to blocking closePolicy.
    // See LingmoUI bug: 454119
    interactive: true

    onPositionChanged: {
        if (!modal && !root.peeking && !root.animating) {
            position = 1;
        }
    }

    background: Rectangle {
        color: LingmoUI.Theme.backgroundColor

        Item {
            parent: root.handle
            anchors.fill: parent

            LingmoUI.ShadowedRectangle {
                id: handleGraphics
                anchors.centerIn: parent

                LingmoUI.Theme.colorSet: parent.parent.handleAnchor && parent.parent.handleAnchor.visible ? parent.parent.handleAnchor.LingmoUI.Theme.colorSet : LingmoUI.Theme.Button

                LingmoUI.Theme.backgroundColor: parent.parent.handleAnchor && parent.parent.handleAnchor.visible ? parent.parent.handleAnchor.LingmoUI.Theme.backgroundColor : undefined

                LingmoUI.Theme.textColor: parent.parent.handleAnchor && parent.parent.handleAnchor.visible ? parent.parent.handleAnchor.LingmoUI.Theme.textColor : undefined

                LingmoUI.Theme.inherit: false
                color: root.handle.pressed ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor

                visible: !parent.parent.handleAnchor || !parent.parent.handleAnchor.visible || root.handle.pressed || (root.modal && root.position > 0)

                shadow.color: Qt.rgba(0, 0, 0, root.handle.pressed ? 0.6 : 0.4)
                shadow.yOffset: 1
                shadow.size: LingmoUI.Units.gridUnit / 2

                width: LingmoUI.Units.iconSizes.smallMedium + LingmoUI.Units.smallSpacing * 2
                height: width
                radius: LingmoUI.Units.cornerRadius
                Behavior on color {
                    ColorAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }
            Loader {
                anchors.centerIn: handleGraphics
                width: height
                height: LingmoUI.Units.iconSizes.smallMedium

                LingmoUI.Theme.colorSet: handleGraphics.LingmoUI.Theme.colorSet
                LingmoUI.Theme.backgroundColor: handleGraphics.LingmoUI.Theme.backgroundColor
                LingmoUI.Theme.textColor: handleGraphics.LingmoUI.Theme.textColor

                asynchronous: true

                source: {
                    let edge = root.edge;
                    if (Qt.application.layoutDirection === Qt.RightToLeft) {
                        if (edge === Qt.LeftEdge) {
                            edge = Qt.RightEdge;
                        } else {
                            edge = Qt.LeftEdge;
                        }
                    }

                    if ((root.handleClosedIcon.source || root.handleClosedIcon.name)
                        && (root.handleOpenIcon.source || root.handleOpenIcon.name)) {
                        return Qt.resolvedUrl("templates/private/GenericDrawerIcon.qml");
                    } else if (edge === Qt.LeftEdge) {
                        return Qt.resolvedUrl("templates/private/MenuIcon.qml");
                    } else if (edge === Qt.RightEdge && root instanceof LingmoUI.ContextDrawer) {
                        return Qt.resolvedUrl("templates/private/ContextIcon.qml");
                    } else {
                        return "";
                    }
                }
                onItemChanged: {
                    if (item) {
                        item.drawer = root;
                        item.color = Qt.binding(() => root.handle.pressed
                            ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor);
                    }
                }
            }
        }

        LingmoUI.Separator {
            id: separator

            LayoutMirroring.enabled: false

            anchors {
                top: root.edge === Qt.TopEdge ? parent.bottom : (root.edge === Qt.BottomEdge ? undefined : parent.top)
                left: root.edge === Qt.LeftEdge ? parent.right : (root.edge === Qt.RightEdge ? undefined : parent.left)
                right: root.edge === Qt.RightEdge ? parent.left : (root.edge === Qt.LeftEdge ? undefined : parent.right)
                bottom: root.edge === Qt.BottomEdge ? parent.top : (root.edge === Qt.TopEdge ? undefined : parent.bottom)
                topMargin: segmentedSeparator.height
            }

            visible: !root.modal

            LingmoUI.Theme.inherit: false
            LingmoUI.Theme.colorSet: LingmoUI.Theme.Header
        }

        Item {
            id: segmentedSeparator

            // an alternative to segmented style is full height
            readonly property bool shouldUseSegmentedStyle: {
                if (root.edge !== Qt.LeftEdge && root.edge !== Qt.RightEdge) {
                    return false;
                }
                if (root.collapsed) {
                    return false;
                }
                // compatible header
                const header = root.header ?? null;
                if (header instanceof T.ToolBar || header instanceof KT.AbstractApplicationHeader) {
                    return true;
                }
                // or compatible content
                if (root.contentItem instanceof ColumnLayout && root.contentItem.children[0] instanceof T.ToolBar) {
                    return true;
                }
                return false;
            }

            anchors {
                top: parent.top
                left: separator.left
                right: separator.right
            }

            height: {
                if (root.edge !== Qt.LeftEdge && root.edge !== Qt.RightEdge) {
                    return 0;
                }
                if (typeof applicationWindow === "undefined") {
                    return 0;
                }
                const window = applicationWindow();
                const globalToolBar = window.pageStack?.globalToolBar;
                if (!globalToolBar) {
                    return 0;
                }

                return globalToolBar.preferredHeight;
            }

            visible: separator.visible

            LingmoUI.Separator {
                LayoutMirroring.enabled: false

                anchors {
                    fill: parent
                    topMargin: segmentedSeparator.shouldUseSegmentedStyle ? LingmoUI.Units.largeSpacing : 0
                    bottomMargin: segmentedSeparator.shouldUseSegmentedStyle ? LingmoUI.Units.largeSpacing : 0
                }

                Behavior on anchors.topMargin {
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                Behavior on anchors.bottomMargin {
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                LingmoUI.Theme.inherit: false
                LingmoUI.Theme.colorSet: LingmoUI.Theme.Header
            }
        }

        KP.EdgeShadow {
            z: -2
            visible: root.modal
            edge: root.edge
            anchors {
                right: root.edge === Qt.RightEdge ? parent.left : (root.edge === Qt.LeftEdge ? undefined : parent.right)
                left: root.edge === Qt.LeftEdge ? parent.right : (root.edge === Qt.RightEdge ? undefined : parent.left)
                top: root.edge === Qt.TopEdge ? parent.bottom : (root.edge === Qt.BottomEdge ? undefined : parent.top)
                bottom: root.edge === Qt.BottomEdge ? parent.top : (root.edge === Qt.TopEdge ? undefined : parent.bottom)
            }

            opacity: root.position === 0 ? 0 : 1

            Behavior on opacity {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
