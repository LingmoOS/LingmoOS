/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Window
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.desktop.private as Private
import org.kde.qqc2desktopstyle.private as StylePrivate

T.ComboBox {
    id: controlRoot

    Kirigami.Theme.colorSet: editable ? Kirigami.Theme.View : Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    baselineOffset: contentItem.y + contentItem.baselineOffset

    hoverEnabled: true
    wheelEnabled: true

    padding: 5
    leftPadding: editable && mirrored ? 24 : padding
    rightPadding: editable && !mirrored ? 24 : padding

    delegate: ItemDelegate {
        required property var model
        required property int index
        width: ListView.view.width
        text: model[controlRoot.textRole]
        highlighted: controlRoot.highlightedIndex === index
        property bool separatorVisible: false
        Kirigami.Theme.colorSet: controlRoot.Kirigami.Theme.inherit ? controlRoot.Kirigami.Theme.colorSet : Kirigami.Theme.View
        Kirigami.Theme.inherit: controlRoot.Kirigami.Theme.inherit
    }

    indicator: Item {}

    /* ensure that the combobox and its popup have enough width for all of its items
     * TODO remove for KF6 because it is fixed by Qt6 */
    onCountChanged: {
        let maxWidth = 75
        for (let i = 0; i < count; ++i) {
            maxWidth = Math.max(maxWidth, fontMetrics.boundingRect(controlRoot.textAt(i)).width)
        }
        styleitem.contentWidth = maxWidth
    }

    FontMetrics {
        id: fontMetrics
    }

    contentItem: T.TextField {
        padding: 0
        text: controlRoot.editable ? controlRoot.editText : controlRoot.displayText

        enabled: controlRoot.editable
        autoScroll: controlRoot.editable
        readOnly: controlRoot.down
        focus: true

        visible: controlRoot.editable
        inputMethodHints: controlRoot.inputMethodHints
        validator: controlRoot.validator

        // Work around Qt bug where NativeRendering breaks for non-integer scale factors
        // https://bugreports.qt.io/browse/QTBUG-67007
        renderType: Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering
        color: controlRoot.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
        selectionColor: Kirigami.Theme.highlightColor
        selectedTextColor: Kirigami.Theme.highlightedTextColor

        selectByMouse: !Kirigami.Settings.tabletMode
        cursorDelegate: Kirigami.Settings.tabletMode ? mobileCursor : null

        font: controlRoot.font
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        opacity: controlRoot.enabled ? 1 : 0.3

        onFocusChanged: {
            if (focus) {
                Private.MobileTextActionsToolBar.controlRoot = this;
            }
        }

        onTextChanged: Private.MobileTextActionsToolBar.shouldBeVisible = false;
        onPressed: Private.MobileTextActionsToolBar.shouldBeVisible = true;

        onPressAndHold: {
            if (!Kirigami.Settings.tabletMode) {
                return;
            }
            forceActiveFocus();
            cursorPosition = positionAt(event.x, event.y);
            selectWord();
        }
    }

    Component {
        id: mobileCursor
        Private.MobileCursor {
            target: controlRoot.contentItem
        }
    }

    Private.MobileCursor {
        target: controlRoot.contentItem
        selectionStartHandle: true
        readonly property rect rect: target.positionToRectangle(target.selectionStart)
        x: rect.x + 5
        y: rect.y + 6
    }

    background: StylePrivate.StyleItem {
        id: styleitem
        control: controlRoot
        elementType: "combobox"
        flat: controlRoot.flat
        anchors.fill: parent
        hover: controlRoot.hovered
        on: controlRoot.down
        hasFocus: controlRoot.activeFocus
        enabled: controlRoot.enabled
        // contentHeight as in QComboBox magic numbers taken from QQC1 style
        contentHeight: Math.max(Math.ceil(textHeight("")), 14) + 2
        text: controlRoot.displayText
        properties: {
            "editable": control.editable
        }
    }

    popup: T.Popup {
        Kirigami.OverlayZStacking.layer: Kirigami.OverlayZStacking.Menu
        z: Kirigami.OverlayZStacking.z

        y: controlRoot.height
        width: controlRoot.width
        implicitHeight: contentItem.implicitHeight
        topMargin: Kirigami.Units.mediumSpacing
        bottomMargin: Kirigami.Units.mediumSpacing
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: controlRoot.Kirigami.Theme.inherit
        modal: true
        dim: true
        closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutside

        // Forces it to have a transparent dimmer.
        // A dimmer is needed for "click outside" to work reliably in some views
        // but default dimmer would, well, dim the contents in pure QtQuick windows,
        // like ApplicationWindow, which we don't want.
        T.Overlay.modal: Item { }

        contentItem: ScrollView {
            LayoutMirroring.enabled: controlRoot.mirrored
            LayoutMirroring.childrenInherit: true

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor
                radius: Kirigami.Units.cornerRadius
            }
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ListView {
                id: listView

                // this causes us to load at least one delegate
                // this is essential in guessing the contentHeight
                // which is needed to initially resize the popup
                cacheBuffer: 1

                clip: height < implicitHeight
                implicitHeight: contentHeight
                model: controlRoot.delegateModel
                delegate: controlRoot.delegate
                currentIndex: controlRoot.highlightedIndex
                highlightRangeMode: ListView.ApplyRange
                highlightMoveDuration: 0
                boundsBehavior: Flickable.StopAtBounds
            }
        }
        background: Kirigami.ShadowedRectangle {
            anchors {
                fill: parent
                margins: -1
            }
            radius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.backgroundColor

            border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
            border.width: 1

            shadow.xOffset: 0
            shadow.yOffset: 2
            shadow.color: Qt.rgba(0, 0, 0, 0.3)
            shadow.size: 8
        }
    }
}
