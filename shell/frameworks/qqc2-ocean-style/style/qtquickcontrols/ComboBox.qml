/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Window
import QtQuick.Templates as T
import QtQuick.Controls as Controls
import org.kde.lingmoui as LingmoUI

import org.kde.ocean.impl as Impl

T.ComboBox {
    id: control

    property real __indicatorMargin: control.indicator && control.indicator.visible && control.indicator.width > 0 ? control.spacing + indicator.width + control.spacing : 0
    property real __widestImplicitContentWidth: implicitContentWidth
    readonly property bool __isContentItemTextInput: contentItem instanceof TextInput

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            __widestImplicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    // palette: LingmoUI.Theme.palette
    LingmoUI.Theme.colorSet: control.editable ? LingmoUI.Theme.View : LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false

    spacing: LingmoUI.Units.mediumSpacing

    hoverEnabled: Qt.styleHints.useHoverEffects

    leftPadding: horizontalPadding + (!control.mirrored ? 0 : __indicatorMargin)
    rightPadding: horizontalPadding + (control.mirrored ? 0 : __indicatorMargin)

    contentItem: Controls.TextField {
        id: textField
        palette: control.palette
        // TextField padding doesn't automatically mirror
        leftPadding: control.mirrored ? 0 : Impl.Units.mediumHorizontalPadding
        rightPadding: !control.mirrored ? 0 : Impl.Units.mediumHorizontalPadding

        text: control.editable ? control.editText : control.displayText

        enabled: control.editable
        autoScroll: control.editable
        readOnly: control.down || !control.editable
        inputMethodHints: control.inputMethodHints
        validator: control.validator

        // Using control.LingmoUI.Theme.textColor instead of directly using
        // LingmoUI.Theme.textColor because the latter always uses the disabled
        // palette when textField.enabled == false
        color: control.LingmoUI.Theme.textColor

        background: null
    }

    indicator: LingmoUI.Icon {
        implicitHeight: LingmoUI.Units.iconSizes.sizeForLabels
        implicitWidth: implicitHeight
        x: control.mirrored ? control.leftPadding : control.leftPadding + control.availableWidth + control.spacing
        y: control.topPadding + (control.availableHeight - height) / 2
        source: "arrow-down"
    }

    background: Impl.ComboBoxBackground {
        control: control

        Rectangle {
            id: separator
            visible: control.editable
            width: Impl.Units.smallBorder
            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom
                rightMargin: (control.mirrored ? control.leftPadding : control.rightPadding) - width
                topMargin: control.topPadding + (__isContentItemTextInput ? control.contentItem.topPadding : 0)
                bottomMargin: control.bottomPadding + (__isContentItemTextInput ? control.contentItem.bottomPadding : 0)
            }

            color: control.down
                || ((control.hovered || control.visualFocus)
                    && !(control.contentItem && control.contentItem.hasOwnProperty("hovered") && control.contentItem.hovered))
                ? LingmoUI.Theme.focusColor : Impl.Theme.separatorColor()

            Behavior on color {
                enabled: control.down || control.hovered
                ColorAnimation {
                    duration: LingmoUI.Units.shortDuration
                    easing.type: Easing.OutCubic
                }
            }
        }

        LingmoUI.ShadowedRectangle {
            id: pressedBg
            property real leftRadius: control.mirrored ? radius : 0
            property real rightRadius: !control.mirrored ? radius : 0
            visible: false

            anchors {
                left: separator.left
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }

            LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
            LingmoUI.Theme.inherit: false
            color: LingmoUI.Theme.alternateBackgroundColor

            radius: parent.radius
            corners {
                topLeftRadius: leftRadius
                topRightRadius: rightRadius
                bottomLeftRadius: leftRadius
                bottomRightRadius: rightRadius
            }

            border.color: LingmoUI.Theme.focusColor
            border.width: Impl.Units.smallBorder

            opacity: 0

            states: State {
                name: "pressed"
                when: control.down && control.editable
                PropertyChanges {
                    target: pressedBg
                    opacity: 1
                    visible: true
                }
            }
            transitions: Transition {
                from: "pressed"
                to: ""
                SequentialAnimation {
                    OpacityAnimator {
                        duration: LingmoUI.Units.shortDuration
                        easing.type: Easing.OutCubic
                    }
                    PropertyAction {
                        target: pressedBg
                        property: "visible"
                        value: false
                    }
                }
            }
        }
    }

    delegate: Controls.MenuItem {
        implicitWidth: leftPadding + implicitContentWidth + rightPadding
        width: parent ? parent.width : implicitWidth
        text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
        __reserveSpaceForIndicator: false
        __reserveSpaceForIcon: false
        __reserveSpaceForArrow: false
    }

    popup: Controls.Menu {
        y: control.height
        x: (control.width - width)/2
        implicitWidth: contentWidth + leftPadding + rightPadding
        width: Math.max(control.width, implicitWidth)
        padding: LingmoUI.Units.smallSpacing

        delegate: null

        contentItem: ListView {
            id: listView
            // this causes us to load at least one delegate
            // this is essential in guessing the contentHeight
            // which is needed to initially resize the popup
            //cacheBuffer: 1
            implicitHeight: contentHeight
            implicitWidth: contentWidth
            model: control.delegateModel // Why isn't this in the ComboBox documentation?
            currentIndex: control.highlightedIndex
            highlightMoveDuration: LingmoUI.Units.shortDuration
            highlightMoveVelocity: LingmoUI.Units.gridUnit * 20
            highlight: Impl.ListViewHighlight {
                currentIndex: control.highlightedIndex
                count: control.count
                alwaysCurveCorners: true
            }
            interactive: Window.window ? contentHeight + control.topPadding + control.bottomPadding > Window.window.height : false
            clip: interactive // Only needed when the ListView can be dragged/flicked
            keyNavigationWraps: true
            ScrollBar.vertical: Controls.ScrollBar {}
        }
    }

    WheelHandler {
        target: control
        // FIXME: shouldDec and shouldInc don't work reliably with touchpads when the popup is open.
        // Sometimes it's fine, sometimes you need to scroll extra hard to move the highlight.
        enabled: control.count > 1 && (popup ? !popup.visible : true)
        // Maybe only handle mouse wheels (default) for now. Touchpad scrolling can be too sensitive sometimes.
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onWheel: {
            // Degrees still change by differences greater than 1 scrollwheel notch (typically 15) if you scroll fast enough.
            // That means multiple scroll ticks are registered before the code can run.
            // Doesn't seem to impact how the control feels much at least.
            let shouldDec = rotation >= 15
            let shouldInc = rotation <= -15
            let shouldReset = (rotation > 0 && control.currentIndex == 0) || (rotation < 0 && control.currentIndex == control.count-1)
            if (shouldDec) {
                control.decrementCurrentIndex(); // This moves the highlight up
                rotation = 0
            } else if (shouldInc) {
                control.incrementCurrentIndex(); // This moves the highlight down
                rotation = 0
            } else if (shouldReset) {
                rotation = 0
            }
        }
    }

    TextMetrics {
        id: textMetrics
    }

    // Mimicks WidestTextWhenCompleted from Qt 6.
    // Maybe use this if app devs are interested.
    // Don't use it for now because lots of GUIs aren't designed for it.
    /*
    Component.onCompleted: {
        // TODO: Remove in Qt 6
        if (// Qt 6 does this in QQuickComboBoxPrivate::calculateWidestTextWidth()
            __isContentItemTextInput
            // Kind of an arbitrary limit, but prevents making lots of calculations with larger models
            && control.count <= 20
        ) {
            let widest = Math.ceil(contentItem.contentWidth)
            for (let i = 0; i < count; ++i) {
                textMetrics.text = control.textAt(i)
                widest = Math.max(widest, textMetrics.width)
            }
            __widestImplicitContentWidth = Math.ceil(widest) + contentItem.leftPadding + contentItem.rightPadding
        }
    }
    */
}
