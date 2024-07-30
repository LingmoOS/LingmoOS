/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Konrad Materka <materka@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.draganddrop as DnD
import org.kde.lingmoui as LingmoUI
import org.kde.kitemmodels as KItemModels
import org.kde.ksvg as KSvg
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid

import "items" as Items

ContainmentItem {
    id: root

    readonly property bool vertical: Plasmoid.formFactor === LingmoCore.Types.Vertical

    Layout.minimumWidth: vertical ? LingmoUI.Units.iconSizes.small : mainLayout.implicitWidth + LingmoUI.Units.smallSpacing
    Layout.minimumHeight: vertical ? mainLayout.implicitHeight + LingmoUI.Units.smallSpacing : LingmoUI.Units.iconSizes.small

    LayoutMirroring.enabled: !vertical && Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    readonly property alias systemTrayState: systemTrayState
    readonly property alias itemSize: tasksGrid.itemSize
    readonly property alias visibleLayout: tasksGrid
    readonly property alias hiddenLayout: expandedRepresentation.hiddenLayout
    readonly property bool oneRowOrColumn: tasksGrid.rowsOrColumns === 1

    readonly property alias hiddenModel: hiddenModel

    Component.onCompleted: {
        // We need all the plasmoiditems to be there for correct working of shortcuts.
        // Instantiators create the plasmoiditems: ensure this is done after
        // this containmentitem actually  exists so they can be immediately parented properly
        // set active and not the model, as this wil lcause an assert deep in Qt
        activeInstantiator.active = true;
        hiddenInstantiator.active = true;
    }

    KItemModels.KSortFilterProxyModel {
        id: activeModel
        sourceModel: Plasmoid.systemTrayModel
        filterRoleName: "effectiveStatus"
        filterRowCallback: (sourceRow, sourceParent) => {
            let value = sourceModel.data(sourceModel.index(sourceRow, 0, sourceParent), filterRole);
            return value === LingmoCore.Types.ActiveStatus;
        }
    }

    KItemModels.KSortFilterProxyModel {
        id: hiddenModel
        sourceModel: Plasmoid.systemTrayModel
        filterRoleName: "effectiveStatus"
        filterRowCallback: (sourceRow, sourceParent) => {
            let value = sourceModel.data(sourceModel.index(sourceRow, 0, sourceParent), filterRole);
            return value === LingmoCore.Types.PassiveStatus
        }
    }

    Instantiator {
        id: hiddenInstantiator
        // It's important that those are inactive at creation time
        // to not create plasmoiditems too soon
        active: false
        model: hiddenModel
        delegate: Connections {
            required property QtObject applet
            required property int row
            target: applet
            function onExpandedChanged(expanded: bool) {
                if (expanded) {
                    systemTrayState.setActiveApplet(applet, row)
                }
            }
        }
    }

    Instantiator {
        id: activeInstantiator
        active: false
        model:activeModel
        delegate: Connections {
            required property QtObject applet
            required property int row
            target: applet
            function onExpandedChanged(expanded: bool) {
                if (expanded) {
                    systemTrayState.setActiveApplet(applet, row)
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        onWheel: wheel => {
            // Don't propagate unhandled wheel events
            wheel.accepted = true;
        }

        SystemTrayState {
            id: systemTrayState
        }

        //being there forces the items to fully load, and they will be reparented in the popup one by one, this item is *never* visible
        Item {
            id: preloadedStorage
            visible: false
        }

        CurrentItemHighLight {
            location: Plasmoid.location
            parent: root
        }

        DnD.DropArea {
            anchors.fill: parent

            preventStealing: true

            /** Extracts the name of the system tray applet in the drag data if present
            * otherwise returns null*/
            function systemTrayAppletName(event) {
                if (event.mimeData.formats.indexOf("text/x-plasmoidservicename") < 0) {
                    return null;
                }
                const plasmoidId = event.mimeData.getDataAsByteArray("text/x-plasmoidservicename");

                if (!Plasmoid.isSystemTrayApplet(plasmoidId)) {
                    return null;
                }
                return plasmoidId;
            }

            onDragEnter: event => {
                if (!systemTrayAppletName(event)) {
                    event.ignore();
                }
            }

            onDrop: event => {
                const plasmoidId = systemTrayAppletName(event);
                if (!plasmoidId) {
                    event.ignore();
                    return;
                }

                if (Plasmoid.configuration.extraItems.indexOf(plasmoidId) < 0) {
                    const extraItems = Plasmoid.configuration.extraItems;
                    extraItems.push(plasmoidId);
                    Plasmoid.configuration.extraItems = extraItems;
                }
            }
        }


        //Main Layout
        GridLayout {
            id: mainLayout

            rowSpacing: 0
            columnSpacing: 0
            anchors.fill: parent

            flow: vertical ? GridLayout.TopToBottom : GridLayout.LeftToRight

            GridView {
                id: tasksGrid

                Layout.alignment: Qt.AlignCenter

                interactive: false //disable features we don't need
                flow: vertical ? GridView.LeftToRight : GridView.TopToBottom

                // The icon size to display when not using the auto-scaling setting
                readonly property int smallIconSize: LingmoUI.Units.iconSizes.smallMedium

                // Automatically use autoSize setting when in tablet mode, if it's
                // not already being used
                readonly property bool autoSize: Plasmoid.configuration.scaleIconsToFit || LingmoUI.Settings.tabletMode

                readonly property int gridThickness: root.vertical ? root.width : root.height
                // Should change to 2 rows/columns on a 56px panel (in standard DPI)
                readonly property int rowsOrColumns: autoSize ? 1 : Math.max(1, Math.min(count, Math.floor(gridThickness / (smallIconSize + LingmoUI.Units.smallSpacing))))

                // Add margins only if the panel is larger than a small icon (to avoid large gaps between tiny icons)
                readonly property int cellSpacing: LingmoUI.Units.smallSpacing * (LingmoUI.Settings.tabletMode ? 6 : Plasmoid.configuration.iconSpacing)
                readonly property int smallSizeCellLength: gridThickness < smallIconSize ? smallIconSize : smallIconSize + cellSpacing

                cellHeight: {
                    if (root.vertical) {
                        return autoSize ? itemSize + (gridThickness < itemSize ? 0 : cellSpacing) : smallSizeCellLength
                    } else {
                        return autoSize ? root.height : Math.floor(root.height / rowsOrColumns)
                    }
                }
                cellWidth: {
                    if (root.vertical) {
                        return autoSize ? root.width : Math.floor(root.width / rowsOrColumns)
                    } else {
                        return autoSize ? itemSize + (gridThickness < itemSize ? 0 : cellSpacing) : smallSizeCellLength
                    }
                }

                //depending on the form factor, we are calculating only one dimension, second is always the same as root/parent
                implicitHeight: root.vertical ? cellHeight * Math.ceil(count / rowsOrColumns) : root.height
                implicitWidth: !root.vertical ? cellWidth * Math.ceil(count / rowsOrColumns) : root.width

                readonly property int itemSize: {
                    if (autoSize) {
                        return LingmoUI.Units.iconSizes.roundedIconSize(Math.min(Math.min(root.width, root.height) / rowsOrColumns, LingmoUI.Units.iconSizes.enormous))
                    } else {
                        return smallIconSize
                    }
                }

                model: activeModel

                delegate: Items.ItemLoader {
                    id: delegate

                    width: tasksGrid.cellWidth
                    height: tasksGrid.cellHeight
                    minLabelHeight: 0

                    // We need to recalculate the stacking order of the z values due to how keyboard navigation works
                    // the tab order depends exclusively from this, so we redo it as the position in the list
                    // ensuring tab navigation focuses the expected items
                    Component.onCompleted: {
                        let item = tasksGrid.itemAtIndex(index - 1);
                        if (item) {
                            Plasmoid.stackItemBefore(delegate, item)
                        } else {
                            item = tasksGrid.itemAtIndex(index + 1);
                        }
                        if (item) {
                            Plasmoid.stackItemAfter(delegate, item)
                        }
                    }
                }
            }

            ExpanderArrow {
                id: expander
                Layout.fillWidth: vertical
                Layout.fillHeight: !vertical
                Layout.alignment: vertical ? Qt.AlignVCenter : Qt.AlignHCenter
                iconSize: tasksGrid.itemSize
                visible: root.hiddenLayout.itemCount > 0
            }
        }

        Timer {
            id: expandedSync
            interval: 100
            onTriggered: systemTrayState.expanded = dialog.visible;
        }

        //Main popup
        LingmoCore.AppletPopup {
            id: dialog
            objectName: "popupWindow"
            visualParent: root
            popupDirection: switch (Plasmoid.location) {
                case LingmoCore.Types.TopEdge:
                    return Qt.BottomEdge
                case LingmoCore.Types.LeftEdge:
                    return Qt.RightEdge
                case LingmoCore.Types.RightEdge:
                    return Qt.LeftEdge
                default:
                    return Qt.TopEdge
            }
            margin: (Plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentPrefersFloatingApplets) ? LingmoUI.Units.largeSpacing : 0

            floating: Plasmoid.location == LingmoCore.Desktop

            removeBorderStrategy: Plasmoid.location === LingmoCore.Types.Floating
            ? LingmoCore.AppletPopup.AtScreenEdges
            : LingmoCore.AppletPopup.AtScreenEdges | LingmoCore.AppletPopup.AtPanelEdges


            hideOnWindowDeactivate: !Plasmoid.configuration.pin
            visible: systemTrayState.expanded
            appletInterface: root

            backgroundHints: (Plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentPrefersOpaqueBackground) ? LingmoCore.AppletPopup.SolidBackground : LingmoCore.AppletPopup.StandardBackground

            onVisibleChanged: {
                if (!visible) {
                    expandedSync.restart();
                } else {
                    if (expandedRepresentation.plasmoidContainer.visible) {
                        expandedRepresentation.plasmoidContainer.forceActiveFocus();
                    } else if (expandedRepresentation.hiddenLayout.visible) {
                        expandedRepresentation.hiddenLayout.forceActiveFocus();
                    }
                }
            }
            mainItem: ExpandedRepresentation {
                id: expandedRepresentation

                Keys.onEscapePressed: event => {
                    systemTrayState.expanded = false
                }

                // Draws a line between the applet dialog and the panel
                KSvg.SvgItem {
                    id: separator
                    // Only draw for popups of panel applets, not desktop applets
                    visible: [LingmoCore.Types.TopEdge, LingmoCore.Types.LeftEdge, LingmoCore.Types.RightEdge, LingmoCore.Types.BottomEdge]
                        .includes(Plasmoid.location) && !dialog.margin
                    anchors {
                        topMargin: -dialog.topPadding
                        leftMargin: -dialog.leftPadding
                        rightMargin: -dialog.rightPadding
                        bottomMargin: -dialog.bottomPadding
                    }
                    z: 999 /* Draw the line on top of the applet */
                    elementId: (Plasmoid.location === LingmoCore.Types.TopEdge || Plasmoid.location === LingmoCore.Types.BottomEdge) ? "horizontal-line" : "vertical-line"
                    imagePath: "widgets/line"
                    // QTBUG-120464: Use AnchorChanges instead of bindings as it's officially supported: https://doc.qt.io/qt-6/qtquick-positioning-anchors.html#changing-anchors
                    states: [
                        State {
                            when: Plasmoid.location === LingmoCore.Types.TopEdge
                            AnchorChanges {
                                target: separator
                                anchors {
                                    top: separator.parent.top
                                    left: separator.parent.left
                                    right: separator.parent.right
                                }
                            }
                            PropertyChanges {
                                target: separator
                                height: 1
                            }
                        },
                        State {
                            when: Plasmoid.location === LingmoCore.Types.LeftEdge
                            AnchorChanges {
                                target: separator
                                anchors {
                                    left: separator.parent.left
                                    top: separator.parent.top
                                    bottom: separator.parent.bottom
                                }
                            }
                            PropertyChanges {
                                target: separator
                                width: 1
                            }
                        },
                        State {
                            when: Plasmoid.location === LingmoCore.Types.RightEdge
                            AnchorChanges {
                                target: separator
                                anchors {
                                    top: separator.parent.top
                                    right: separator.parent.right
                                    bottom: separator.parent.bottom
                                }
                            }
                            PropertyChanges {
                                target: separator
                                width: 1
                            }
                        },
                        State {
                            when: Plasmoid.location === LingmoCore.Types.BottomEdge
                            AnchorChanges {
                                target: separator
                                anchors {
                                    left: separator.parent.left
                                    right: separator.parent.right
                                    bottom: separator.parent.bottom
                                }
                            }
                            PropertyChanges {
                                target: separator
                                height: 1
                            }
                        }
                    ]
                }

                LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
                LayoutMirroring.childrenInherit: true
            }
        }
    }
}
