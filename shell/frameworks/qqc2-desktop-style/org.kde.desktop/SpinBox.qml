/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Window
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.qqc2desktopstyle.private as StylePrivate

T.SpinBox {
    id: controlRoot
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    implicitWidth: Math.max(styleitem.fullRectSizeHint.width,
                            implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(styleitem.fullRectSizeHint.height,
                             implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 6
    leftPadding: controlRoot.mirrored ? ___rPadding : ___lPadding
    rightPadding: controlRoot.mirrored ? ___lPadding : ___rPadding

    readonly property int ___lPadding: styleitem.upRectSizeHint.x === styleitem.downRectSizeHint.x ? horizontalPadding : styleitem.downRectSizeHint.width
    readonly property int ___rPadding: styleitem.upRectSizeHint.x === styleitem.downRectSizeHint.x ? styleitem.downRectSizeHint.width : styleitem.upRectSizeHint.width

    hoverEnabled: true
    wheelEnabled: true
    editable: true

    validator: IntValidator {
        locale: controlRoot.locale.name
        bottom: Math.min(controlRoot.from, controlRoot.to)
        top: Math.max(controlRoot.from, controlRoot.to)
    }

    inputMethodHints: Qt.ImhFormattedNumbersOnly

    contentItem: T.TextField {
        readonly property TextMetrics _textMetrics: TextMetrics {
            text: controlRoot.textFromValue(controlRoot.to, controlRoot.locale)
            font: controlRoot.font
        }
        // Sometimes textMetrics.width isn't as wide as the actual text; add 2
        implicitWidth: Math.max(_textMetrics.width + 2, Math.round(contentWidth))
            + leftPadding + rightPadding
        implicitHeight: Math.round(contentHeight) + topPadding + bottomPadding
        z: 2
        font: controlRoot.font
        palette: controlRoot.palette
        text: controlRoot.textFromValue(controlRoot.value, controlRoot.locale)
        color: Kirigami.Theme.textColor
        selectionColor: Kirigami.Theme.highlightColor
        selectedTextColor: Kirigami.Theme.highlightedTextColor
        selectByMouse: true
        hoverEnabled: false // let hover events propagate to SpinBox
        verticalAlignment: Qt.AlignVCenter
        readOnly: !controlRoot.editable
        validator: controlRoot.validator
        inputMethodHints: controlRoot.inputMethodHints

        // SpinBox does not update its value during editing, see QTBUG-91281
        onTextEdited: if (controlRoot.contentItem.text.length > 0 && acceptableInput) {
            controlRoot.value = controlRoot.valueFromText(controlRoot.contentItem.text, controlRoot.locale)
            controlRoot.valueModified()
        }

        // Since the contentItem receives focus (we make them editable by default),
        // the screen reader reads its Accessible properties instead of the SpinBox's
        Accessible.name: controlRoot.Accessible.name
        Accessible.description: controlRoot.Accessible.description
    }

    up.indicator: Item {
        implicitWidth: styleitem.upRect.width
        implicitHeight: styleitem.upRect.height

        x: styleitem.upRect.x
        y: styleitem.upRect.y
    }
    down.indicator: Item {
        implicitWidth: styleitem.downRect.width
        implicitHeight: styleitem.downRect.height

        x: styleitem.downRect.x
        y: styleitem.downRect.y
    }

    background: StylePrivate.StyleItem {
        id: styleitem
        control: controlRoot
        elementType: "spinbox"
        anchors.fill: parent
        hover: controlRoot.hovered
        hasFocus: controlRoot.activeFocus
        enabled: controlRoot.enabled

        // static hints calculated once for minimum sizes
        property rect upRectSizeHint: styleitem.subControlRect("up")
        property rect downRectSizeHint: styleitem.subControlRect("down")
        property rect editRectSizeHint: styleitem.subControlRect("edit")
        property rect fullRectSizeHint: styleitem.computeBoundingRect([upRectSizeHint, downRectSizeHint, editRectSizeHint])

        // dynamic hints calculated every resize to keep the buttons in place
        property rect upRect: upRectSizeHint
        property rect downRect: downRectSizeHint

        function recompute() {
            upRect = styleitem.subControlRect("up")
            downRect = styleitem.subControlRect("down")
        }

        onWidthChanged: recompute()
        onHeightChanged: recompute()

        value: {
            var value = 0;
            if (controlRoot.up.hovered || controlRoot.up.pressed) {
                value |= 1 << 0;
            }
            if (controlRoot.down.hovered || controlRoot.down.pressed) {
                value |= 1 << 1;
            }
            if (controlRoot.value !== controlRoot.to) {
                value |= 1 << 2;
            }
            if (controlRoot.value !== controlRoot.from) {
                value |= 1 << 3;
            }
            return value;
        }
    }
}
