/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.desktop.private as Private

T.MenuItem {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding + (arrow ? arrow.implicitWidth : 0))

    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    baselineOffset: contentItem.y + contentItem.baselineOffset

    // width is set in https://invent.kde.org/qt/qt/qtdeclarative/-/blob/1e6cb2462ee87476a5eab7c71735c001a46c7b55/src/quicktemplates/qquickmenu.cpp#L332
    // Note: Binding height here to make sure menu items that are not visible are
    // properly collapsed, otherwise they will still occupy space inside the menu.
    height: visible ? undefined : 0

    padding: Kirigami.Units.smallSpacing
    verticalPadding: Kirigami.Settings.hasTransientTouchInput ? 8 : 4 // Hardcoded to the Breeze theme value
    hoverEnabled: !Kirigami.Settings.isMobile

    Kirigami.MnemonicData.enabled: enabled && visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.MenuItem
    Kirigami.MnemonicData.label: text
    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))
        sequence: controlRoot.Kirigami.MnemonicData.sequence
        onActivated: {
            if (controlRoot.checkable) {
                controlRoot.toggle();
            } else {
                controlRoot.clicked();
            }
        }
    }

    contentItem: RowLayout {
        Item {
            Layout.preferredWidth: (controlRoot.ListView.view && controlRoot.ListView.view.hasCheckables) || controlRoot.checkable ? controlRoot.indicator.width : Kirigami.Units.smallSpacing
        }
        Kirigami.Icon {
            Layout.alignment: Qt.AlignVCenter
            visible: (controlRoot.ListView.view && controlRoot.ListView.view.hasIcons)
                || (controlRoot.icon.name !== "" || controlRoot.icon.source.toString() !== "")
            source: controlRoot.icon.name !== "" ? controlRoot.icon.name : controlRoot.icon.source
            color: controlRoot.icon.color
            Layout.preferredHeight: Kirigami.Settings.hasTransientTouchInput ? Kirigami.Units.iconSizes.smallMedium : Kirigami.Units.iconSizes.small
            Layout.preferredWidth: Layout.preferredHeight
        }
        Label {
            id: label
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            text: controlRoot.Kirigami.MnemonicData.richTextLabel
            font: controlRoot.font
            color: Kirigami.Theme.textColor
            elide: Text.ElideRight
            visible: controlRoot.text
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
        Label {
            id: shortcut
            Layout.alignment: Qt.AlignVCenter
            visible: controlRoot.action && controlRoot.action.shortcut !== undefined

            Shortcut {
                id: itemShortcut
                sequence: (shortcut.visible && controlRoot.action !== null) ? controlRoot.action.shortcut : ""
            }

            text: visible ? itemShortcut.nativeText : ""
            font: controlRoot.font
            color: label.color
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }
        Item {
            Layout.preferredWidth: Kirigami.Units.smallSpacing
        }
    }

    arrow: Kirigami.Icon {
        x: controlRoot.mirrored ? controlRoot.padding : controlRoot.width - width - controlRoot.padding
        y: controlRoot.topPadding + (controlRoot.availableHeight - height) / 2
        source: controlRoot.mirrored ? "go-next-symbolic-rtl" : "go-next-symbolic"
        width: Kirigami.Units.iconSizes.small
        height: width
        visible: controlRoot.subMenu
    }

    indicator: Private.CheckIndicator {
        x: controlRoot.mirrored ? controlRoot.width - width - controlRoot.rightPadding : controlRoot.leftPadding
        y: controlRoot.topPadding + (controlRoot.availableHeight - height) / 2

        drawIcon: false // We're drawing it ourselves in this control

        visible: controlRoot.checkable
        on: controlRoot.checked
        control: controlRoot
    }

    background: Rectangle {
        implicitWidth: Kirigami.Units.gridUnit * 8
        opacity: (controlRoot.highlighted || controlRoot.hovered) ? 1 : 0
        color: Qt.alpha(Kirigami.Theme.focusColor, 0.3)
        border.color: Kirigami.Theme.focusColor
        border.width: 1
        radius: Kirigami.Units.cornerRadius
    }
}
