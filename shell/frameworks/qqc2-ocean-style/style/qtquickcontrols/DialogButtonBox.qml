/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import org.kde.ocean.impl as Impl

T.DialogButtonBox {
    id: control

    readonly property bool __isHeader: control.position === T.DialogButtonBox.Header
    readonly property bool __isFooter: control.position === T.DialogButtonBox.Footer

    // Children of QML Popup elements and subclasses of Popup are actually
    // children of an internal QQuickPopupItem, a subclass of QQuickPage.
    property bool __isInPopup: parent ? parent.toString().includes("QQuickPopupItem") : false

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    spacing: LingmoUI.Units.smallSpacing

    padding: LingmoUI.Units.smallSpacing

    alignment: Qt.AlignRight

    delegate: Button {
        width: Math.round(Math.min(implicitWidth, (control.availableWidth / control.count) - (control.spacing * (control.count-1))))
        LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.DialogButton
    }

    contentItem: ListView {
        pixelAligned: true
        model: control.contentModel
        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
    }

    background: LingmoUI.ShadowedRectangle {
        property real topRadius: control.__isHeader ? radius : 0
        property real bottomRadius: control.__isFooter ? radius : 0
        radius: control.__isInPopup ? Impl.Units.smallRadius : 0
        corners {
            topLeftRadius: topRadius
            topRightRadius: topRadius
            bottomLeftRadius: bottomRadius
            bottomRightRadius: bottomRadius
        }
        // Enough height for Buttons/ComboBoxes/TextFields with smallSpacing padding on top and bottom
        implicitHeight: Impl.Units.mediumControlHeight + (LingmoUI.Units.smallSpacing * 2)
        color: control.__isInPopup ? "transparent" : LingmoUI.Theme.backgroundColor
    }

    // Standard buttons are destroyed and then recreated every time
    // the `standardButtons` property changes, so it is necessary to
    // run this code every time standardButtonsChanged() is emitted.
    // See QQuickDialogButtonBox::setStandardButtons()
    onStandardButtonsChanged: {
        // standardButton() returns a pointer to an existing standard button.
        // If no such button exists, it returns nullptr.
        // Icon names are copied from KStyle::standardIcon()
        function setStandardIcon(buttonType, iconName) {
            let button = standardButton(buttonType)
            // For some reason, `== ""` works, but `=== ""` and `!name && !source` doesn't.
            if (button && button.icon.name == "" && button.icon.source == "") {
                button.icon.name = iconName
            }
        }
        setStandardIcon(T.Dialog.Ok, "dialog-ok")
        setStandardIcon(T.Dialog.Save, "document-save")
        setStandardIcon(T.Dialog.SaveAll, "document-save-all")
        setStandardIcon(T.Dialog.Open, "document-open")
        setStandardIcon(T.Dialog.Yes, "dialog-ok-apply")
        setStandardIcon(T.Dialog.YesToAll, "dialog-ok")
        setStandardIcon(T.Dialog.No, "dialog-cancel")
        setStandardIcon(T.Dialog.NoToAll, "dialog-cancel")
        setStandardIcon(T.Dialog.Abort, "dialog-cancel")
        setStandardIcon(T.Dialog.Retry, "view-refresh")
        setStandardIcon(T.Dialog.Ignore, "dialog-cancel")
        setStandardIcon(T.Dialog.Close, "dialog-close")
        setStandardIcon(T.Dialog.Cancel, "dialog-cancel")
        setStandardIcon(T.Dialog.Discard, "edit-delete")
        setStandardIcon(T.Dialog.Help, "help-contents")
        setStandardIcon(T.Dialog.Apply, "dialog-ok-apply")
        setStandardIcon(T.Dialog.Reset, "edit-undo")
        setStandardIcon(T.Dialog.RestoreDefaults, "document-revert")
    }
}
