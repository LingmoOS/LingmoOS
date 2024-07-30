/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs as QtDialogs

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils
import org.kde.config as KConfig
import org.kde.kwindowsystem

import org.kde.plasma.private.colorpicker as ColorPicker
import "logic.js" as Logic

PlasmoidItem {
    id: root

    readonly property bool isVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property color recentColor: historyModel.count > 0 ? historyModel.get(0).color : "#00000000" // transparent as fallback
    readonly property string defaultFormat: Plasmoid.configuration.defaultFormat
    readonly property int maxColorCount: 9

    preferredRepresentation: compactRepresentation

    function addColorToHistory(color) {
        // this .toString() is needed otherwise Qt completely screws it up
        // replacing *all* items in the list by the new items and other nonsense
        historyModel.insert(0, { color: color.toString() });
        // limit to 9 entries
        if (historyModel.count > maxColorCount) {
            historyModel.remove(maxColorCount);
        }
        historyModel.save();
    }

    function colorPicked(color) {
        if (color != recentColor) {
            addColorToHistory(color)
        }

        if (Plasmoid.configuration.autoClipboard) {
            picker.copyToClipboard(Logic.formatColor(color, root.defaultFormat))
        }
    }

    function pickColor() {
        root.expanded = false
        picker.pick()
    }

    ColorPicker.GrabWidget {
        id: picker
        onCurrentColorChanged: colorPicked(currentColor)
    }

    Component {
        id: colorWindowComponent

        Window { // QTBUG-119055
            id: window
            width: Kirigami.Units.gridUnit * 19
            height: Kirigami.Units.gridUnit * 23
            maximumWidth: width
            maximumHeight: height
            minimumWidth: width
            minimumHeight: height
            visible: true
            title: Plasmoid.title
            QtDialogs.ColorDialog {
                id: colorDialog
                title: Plasmoid.title
                selectedColor: historyModel.count > 0 ? root.recentColor : undefined /* Prevent transparent colors */
                onAccepted: {
                    root.colorPicked(selectedColor);
                    window.destroy();
                }
                onRejected: window.destroy()
            }
            onClosing: destroy()
            Component.onCompleted: colorDialog.open()
        }
    }

    // prevents the popup from actually opening, needs to be queued
    Timer {
        id: delayedPickTimer
        interval: 0
        onTriggered: root.pickColor()
    }

    ListModel {
        id: historyModel

        function save() {
            let history = [];
            for (let i = 0; i < count; i++) {
               history.push(get(i).color);
            }
            Plasmoid.configuration.history = history;
        }
    }

    Plasmoid.onActivated: {
        if (Plasmoid.configuration.pickOnActivate) {
            delayedPickTimer.start();
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("@action", "Open Color Dialog")
            icon.name: "color-management"
            onTriggered: colorWindowComponent.createObject(root)
        },
        PlasmaCore.Action {
            text: i18nc("@action", "Clear History")
            icon.name: "edit-clear-history"
            onTriggered: {
                historyModel.clear();
                historyModel.save();
            }
        },
        PlasmaCore.Action {
            text: i18nc("@action", "View History")
            icon.name: "view-history"
            onTriggered: root.expanded = true
        }
    ]

    Component.onCompleted: {
        Plasmoid.configuration.history.forEach(item => historyModel.append({ color: item }));
        Logic.copyToClipboardText = i18ndc("plasma_applet_org.kde.plasma.colorpicker", "@title:menu", "Copy to Clipboard"); // i18n is not supported in js library
    }

    compactRepresentation: CompactRepresentation { }

    fullRepresentation: GridView {
        id: fullRoot

        readonly property int columns: 3

        Layout.minimumWidth: columns * Kirigami.Units.gridUnit * 6
        Layout.minimumHeight: Layout.minimumWidth
        Layout.maximumWidth: Layout.minimumWidth
        Layout.maximumHeight: Layout.minimumHeight

        cellWidth: Math.floor(fullRoot.width / fullRoot.columns)
        cellHeight: cellWidth
        boundsBehavior: Flickable.StopAtBounds

        model: historyModel

        highlight: PlasmaExtras.Highlight {}
        highlightMoveDuration: 0

        Loader {
            width: parent.width - Kirigami.Units.gridUnit * 2
            anchors.centerIn: parent
            visible: active

            active: fullRoot.count === 0 && root.expanded
            asynchronous: true

            sourceComponent: PlasmaExtras.PlaceholderMessage {
                id: emptyHint

                opacity: 0
                iconName: "edit-none"

                readonly property bool compositingActive: KWindowSystem.isPlatformWayland || KX11Extras.compositingActive

                text: compositingActive ? i18nc("@info:usagetip", "No colors") : i18nc("@info:status when color picking is unavailable", "Color picking unavailable when compositing is disabled")
                explanation: compositingActive ? "" : i18nc("@info:status when color pick is unavailable", "Compositing has been manually disabled or blocked by a running application")

                helpfulAction: compositingActive ? pickColorAction : enableCompositingAction

                QQC2.Action {
                    id: pickColorAction
                    icon.name: "color-picker"
                    text: i18nc("@action:button", "Pick Color")
                    onTriggered: root.pickColor()
                }

                QQC2.Action {
                    id: enableCompositingAction
                    enabled: KConfig.KAuthorized.authorizeControlModule("kwincompositing")
                    icon.name: "settings-configure"
                    text: i18nc("@action:button open kwincompositing KCM", "Configure Compositing")
                    onTriggered: KCMUtils.KCMLauncher.openSystemSettings("kwincompositing")
                }

                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.OutCubic
                    property: "opacity"
                    running: true
                    target: emptyHint
                    to: 1
                }
            }
        }

        Connections {
            target: root
            function onExpandedChanged() {
                if (root.expanded) {
                    fullRoot.forceActiveFocus()
                }
            }
        }

        Keys.onPressed: event => {
            if (event.key === Qt.Key_Escape) {
                root.expanded = false;
                event.accepted = true;
            }
        }

        // This item serves as a drag pixmap and is captured when a drag starts
        Rectangle {
            id: dragImageDummy
            border {
                color: Kirigami.Theme.textColor
                width: 1
            }
            radius: width
            width: Kirigami.Units.iconSizes.large
            height: Kirigami.Units.iconSizes.large
            visible: false
        }

        delegate: MouseArea {
            id: delegateMouse

            readonly property color currentColor: model.color

            width: fullRoot.cellWidth
            height: fullRoot.cellHeight

            drag.target: rect
            Drag.dragType: Drag.Automatic
            Drag.active: delegateMouse.drag.active
            Drag.mimeData: {
                "application/x-color": rect.color,
                "text/plain": colorLabel.text
            }

            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true

            Keys.onDeletePressed: event => remove()
            Keys.onPressed: event => {
                switch (event.key) {
                case Qt.Key_Space:
                case Qt.Key_Enter:
                case Qt.Key_Return:
                case Qt.Key_Select:
                    copy();
                    break;
                case Qt.Key_Menu:
                    openMenu();
                    break;
                }
            }

            Accessible.name: colorLabel.text
            Accessible.role: Accessible.ButtonMenu

            onContainsMouseChanged: {
                if (containsMouse) {
                    fullRoot.currentIndex = index;
                } else if (fullRoot.currentIndex === index) {
                    fullRoot.currentIndex = -1;
                }
            }

            onPressed: mouse => {
                // grab pixmap only once
                if (Drag.imageSource.toString() === "") { // cannot just do !Drag.imageSource on QUrl
                    dragImageDummy.color = currentColor;
                    dragImageDummy.grabToImage(result => {
                        Drag.imageSource = result.url;
                    });
                }
            }

            onClicked: mouse => {
                if (mouse.button === Qt.LeftButton) {
                    copy();
                } else if (mouse.button === Qt.RightButton) {
                    openMenu();
                }
            }

            function copy() {
                picker.copyToClipboard(Logic.formatColor(currentColor, root.defaultFormat))
                colorLabel.visible = false;
                copyIndicatorLabel.visible = true;
                colorLabelRestoreTimer.start()
            }

            function openMenu() {
                const menu = Logic.createContextMenu(this, currentColor, picker, colorLabel, copyIndicatorLabel, colorLabelRestoreTimer);
                menu.openRelative();
            }

            function remove() {
                historyModel.remove(index);
                historyModel.save();
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                active: colorLabel.truncated
                mainText: colorLabel.text
            }

            Timer {
                id: colorLabelRestoreTimer
                interval: Kirigami.Units.humanMoment
                onTriggered: {
                    colorLabel.visible = true;
                    copyIndicatorLabel.visible = false;
                }
            }

            Rectangle {
                id: rect

                anchors {
                    fill: parent
                    margins: Kirigami.Units.smallSpacing
                }

                color: delegateMouse.currentColor

                border {
                    color: Kirigami.Theme.textColor
                    width: 1
                }

                Rectangle {
                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                        margins: rect.border.width
                    }
                    height: colorLabel.contentHeight + 2 * Kirigami.Units.smallSpacing
                    color: Kirigami.Theme.backgroundColor
                    opacity: 0.8

                    PlasmaComponents3.Label {
                        id: colorLabel
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideLeft
                        fontSizeMode: Text.HorizontalFit
                        minimumPointSize: Kirigami.Theme.smallFont.pointSize
                        text: Logic.formatColor(delegateMouse.currentColor, root.defaultFormat)
                        textFormat: Text.PlainText
                    }

                    PlasmaComponents3.Label {
                        id: copyIndicatorLabel
                        visible: false
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideLeft
                        fontSizeMode: Text.HorizontalFit
                        minimumPointSize: Kirigami.Theme.smallFont.pointSize
                        text: i18nc("@info:progress just copied a color to clipboard", "Copied!")
                        textFormat: Text.PlainText
                    }
                }
            }

            Loader {
                active: parent.containsMouse || Kirigami.Settings.tabletMode || Kirigami.Settings.hasTransientTouchInput
                anchors.right: parent.right
                anchors.top: parent.top
                sourceComponent: PlasmaComponents3.Button {
                    text: i18nc("@action:button", "Delete")
                    icon.name: "delete"
                    display: PlasmaComponents3.AbstractButton.IconOnly

                    onClicked: delegateMouse.remove()

                    PlasmaComponents3.ToolTip {
                        text: parent.text
                    }
                }
            }
        }
    }
}
