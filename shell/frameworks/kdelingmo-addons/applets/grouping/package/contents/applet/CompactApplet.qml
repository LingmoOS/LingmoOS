/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.kquickcontrolsaddons 2.0
import org.kde.lingmo.plasmoid

LingmoCore.ToolTipArea {
    id: root
    objectName: "org.kde.desktop-CompactApplet"
    anchors.fill: parent

    icon: plasmoid.icon
    mainText: plasmoid.toolTipMainText
    subText: plasmoid.toolTipSubText
    location: plasmoid.location
    active: !plasmoidItem.expanded
    textFormat: plasmoid.toolTipTextFormat
    mainItem: plasmoid.toolTipItem ? plasmoid.toolTipItem : null
    property PlasmoidItem plasmoidItem

    property Item fullRepresentation
    property Item compactRepresentation

    onCompactRepresentationChanged: {
        if (compactRepresentation) {
            compactRepresentation.parent = root;
            compactRepresentation.anchors.fill = root;
            compactRepresentation.visible = true;
        }
        root.visible = true;
    }

    onFullRepresentationChanged: {

        if (!fullRepresentation) {
            return;
        }
        //if the fullRepresentation size was restored to a stored size, or if is dragged from the desktop, restore popup size
        if (fullRepresentation.width > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.width
            })
        } else if (fullRepresentation.Layout && fullRepresentation.Layout.preferredWidth > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.Layout.preferredWidth
            })
        } else if (fullRepresentation.implicitWidth > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.implicitWidth
            })
        } else {
            popupWindow.mainItem.width = Qt.binding(function() {
                return LingmoUI.Theme.gridUnit * 35
            })
        }

        if (fullRepresentation.height > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.height
            })
        } else if (fullRepresentation.Layout && fullRepresentation.Layout.preferredHeight > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.Layout.preferredHeight
            })
        } else if (fullRepresentation.implicitHeight > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.implicitHeight
            })
        } else {
            popupWindow.mainItem.height = Qt.binding(function() {
                return LingmoUI.Theme.gridUnit * 25
            })
        }

        fullRepresentation.parent = appletParent;
        fullRepresentation.anchors.fill = fullRepresentation.parent;
    }

    Timer {
        id: expandedSync
        interval: 100
        onTriggered: plasmoidItem.expanded = popupWindow.visible;
    }

    Connections {
        target: plasmoid.internalAction("configure")
        function onTriggered() {
            plasmoidItem.expanded = false
        }
    }

    Connections {
        target: plasmoid
        function onContextualActionsAboutToShow() {
            root.hideToolTip()
        }
    }

    LingmoCore.Dialog {
        id: popupWindow
        objectName: "popupWindow"
        flags: Qt.WindowStaysOnTopHint
        visible: plasmoidItem.expanded && fullRepresentation
        visualParent: compactRepresentation ? compactRepresentation : null
        location: plasmoid.location
        hideOnWindowDeactivate: root.plasmoidItem.hideOnWindowDeactivate

        property var oldStatus: LingmoCore.Types.UnknownStatus

        //It's a MouseEventListener to get all the events, so the eventfilter will be able to catch them
        mainItem: MouseEventListener {
            id: appletParent

            focus: true

            Keys.onEscapePressed: {
                plasmoidItem.expanded = false;
            }

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            Layout.minimumWidth: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.minimumWidth : 0
            Layout.minimumHeight: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.minimumHeight: 0
            Layout.maximumWidth: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.maximumWidth : Infinity
            Layout.maximumHeight: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.maximumHeight: Infinity

            onActiveFocusChanged: {
                if (activeFocus && fullRepresentation) {
                    fullRepresentation.forceActiveFocus()
                }
            }
        }

        onVisibleChanged: {
            if (!visible) {
                expandedSync.restart();
                plasmoid.status = oldStatus;
            } else {
                oldStatus = plasmoid.status;
                plasmoid.status = LingmoCore.Types.RequiresAttentionStatus;
                // This call currently fails and complains at runtime:
                // QWindow::setWindowState: QWindow::setWindowState does not accept Qt::WindowActive
                popupWindow.requestActivate();
            }
        }
    }
}
