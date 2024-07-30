/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Konrad Materka <materka@gmail.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid

LingmoCore.ToolTipArea {
    id: abstractItem

    required property int index
    required property var model

    required property real minLabelHeight

    required property string itemId
    /*required*/ property alias text: label.text

    // subclasses need to bind these tooltip properties
    required mainText
    required subText
    required textFormat

    readonly property alias labelHeight: label.implicitHeight
    readonly property alias iconContainer: iconContainer
    readonly property int /*LingmoCore.Types.ItemStatus*/ status: model.status || LingmoCore.Types.UnknownStatus
    readonly property int /*LingmoCore.Types.ItemStatus*/ effectiveStatus: model.effectiveStatus || LingmoCore.Types.UnknownStatus
    readonly property bool inHiddenLayout: effectiveStatus === LingmoCore.Types.PassiveStatus
    readonly property bool inVisibleLayout: effectiveStatus === LingmoCore.Types.ActiveStatus

    property bool effectivePressed: false

    // input agnostic way to trigger the main action
    signal activated(var pos)

    // proxy signals for MouseArea
    signal clicked(var mouse)
    signal pressed(var mouse)
    signal wheel(var wheel)
    signal contextMenu(var mouse)

    location: {
        if (inHiddenLayout) {
            if (LayoutMirroring.enabled && Plasmoid.location !== LingmoCore.Types.RightEdge) {
                return LingmoCore.Types.LeftEdge;
            } else if (Plasmoid.location !== LingmoCore.Types.LeftEdge) {
                return LingmoCore.Types.RightEdge;
            }
        }

        return Plasmoid.location;
    }

    PulseAnimation {
        targetItem: iconContainer
        running: (abstractItem.status === LingmoCore.Types.NeedsAttentionStatus
                || abstractItem.status === LingmoCore.Types.RequiresAttentionStatus)
            && LingmoUI.Units.longDuration > 0
    }

    MouseArea {
        id: mouseArea
        propagateComposedEvents: true
        // This needs to be above applets when it's in the grid hidden area
        // so that it can receive hover events while the mouse is over an applet,
        // but below them on regular systray, so collapsing works
        z: inHiddenLayout ? 1 : 0
        anchors.fill: abstractItem
        hoverEnabled: true
        drag.filterChildren: true
        // Necessary to make the whole delegate area forward all mouse events
        acceptedButtons: Qt.AllButtons
        // Using onPositionChanged instead of onEntered because changing the
        // index in a scrollable view also changes the view position.
        // onEntered will change the index while the items are scrolling,
        // making it harder to scroll.
        onPositionChanged: if (inHiddenLayout) {
            root.hiddenLayout.currentIndex = index
        }
        onClicked: mouse => { abstractItem.clicked(mouse) }
        onPressed: mouse => {
            if (inHiddenLayout) {
                root.hiddenLayout.currentIndex = index
            }
            abstractItem.hideImmediately()
            abstractItem.pressed(mouse)
        }
        onPressAndHold: mouse => {
            if (mouse.button === Qt.LeftButton) {
                abstractItem.contextMenu(mouse)
            }
        }
        onWheel: wheel => {
            abstractItem.wheel(wheel);
            //Don't accept the event in order to make the scrolling by mouse wheel working
            //for the parent scrollview this icon is in.
            wheel.accepted = false;
        }
    }

    ColumnLayout {
        anchors.fill: abstractItem
        spacing: 0

        FocusScope {
            id: iconContainer
            scale: (abstractItem.effectivePressed || mouseArea.containsPress) ? 0.8 : 1

            activeFocusOnTab: true
            focus: true // Required in HiddenItemsView so keyboard events can be forwarded to this item
            Accessible.name: abstractItem.text
            Accessible.description: abstractItem.subText
            Accessible.role: Accessible.Button
            Accessible.onPressAction: abstractItem.activated(Plasmoid.popupPosition(iconContainer, iconContainer.width/2, iconContainer.height/2));

            Behavior on scale {
                ScaleAnimator {
                    duration: LingmoUI.Units.longDuration
                    easing.type: (effectivePressed || mouseArea.containsPress) ? Easing.OutCubic : Easing.InCubic
                }
            }

            Keys.onPressed: event => {
                switch (event.key) {
                    case Qt.Key_Space:
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Select:
                        abstractItem.activated(Qt.point(width/2, height/2));
                        break;
                    case Qt.Key_Menu:
                        abstractItem.contextMenu(null);
                        event.accepted = true;
                        break;
                }
            }

            property alias container: abstractItem
            property alias inVisibleLayout: abstractItem.inVisibleLayout
            readonly property int size: abstractItem.inVisibleLayout ? root.itemSize : LingmoUI.Units.iconSizes.medium

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: abstractItem.inHiddenLayout ? true : false
            implicitWidth: root.vertical && abstractItem.inVisibleLayout ? abstractItem.width : size
            implicitHeight: !root.vertical && abstractItem.inVisibleLayout ? abstractItem.height : size
            Layout.topMargin: abstractItem.inHiddenLayout ? LingmoUI.Units.mediumSpacing : 0
        }
        LingmoComponents3.Label {
            id: label

            Layout.fillWidth: true
            Layout.fillHeight: abstractItem.inHiddenLayout ? true : false
            //! Minimum required height for all labels is used in order for all
            //! labels to be aligned properly at all items. At the same time this approach does not
            //! enforce labels with 3 lines at all cases so translations that require only one or two
            //! lines will always look consistent with no too much padding
            Layout.minimumHeight: abstractItem.inHiddenLayout ? abstractItem.minLabelHeight : 0
            Layout.leftMargin: abstractItem.inHiddenLayout ? LingmoUI.Units.smallSpacing : 0
            Layout.rightMargin: abstractItem.inHiddenLayout ? LingmoUI.Units.smallSpacing : 0
            Layout.bottomMargin: abstractItem.inHiddenLayout ? LingmoUI.Units.smallSpacing : 0

            visible: abstractItem.inHiddenLayout

            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            maximumLineCount: 3

            opacity: visible ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
