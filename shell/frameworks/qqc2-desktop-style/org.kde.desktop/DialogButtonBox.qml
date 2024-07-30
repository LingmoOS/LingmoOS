/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.qqc2desktopstyle.private as StylePrivate

T.DialogButtonBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    contentWidth: (contentItem as ListView)?.contentWidth ?? 0

    spacing: Kirigami.Units.mediumSpacing
    padding: Kirigami.Units.largeSpacing
    alignment: Qt.AlignRight

    property Item __style: StylePrivate.StyleItem {
        id: styleItem
    }

    delegate: Button {
        // Round because fractional width values are possible.
        width: Math.floor(Math.min(
            implicitWidth,
            // Divide availableWidth (width - leftPadding - rightPadding) by the number of buttons,
            // then subtract the spacing between each button.
            ((control.availableWidth - (control.spacing * (control.count - 1))) / control.count)
        ))
        Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.DialogButton
    }

    contentItem: ListView {
        implicitWidth: contentWidth
        implicitHeight: 32

        model: control.contentModel
        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
    }

    // Standard buttons are destroyed and then recreated every time
    // the `standardButtons` property changes, so it is necessary to
    // run this code every time standardButtonsChanged() is emitted.
    // See QQuickDialogButtonBox::setStandardButtons()
    onStandardButtonsChanged: {
        // standardButton() returns a pointer to an existing standard button.
        // If no such button exists, it returns nullptr.
        // Icon names are copied from KStyle::standardIcon()

        if (!styleItem.styleHint("dialogButtonsHaveIcons")) {
            return;
        }

        function setStandardIcon(buttonType, iconName) {
            const button = standardButton(buttonType)
            if (button && button.icon.name === "" && button.icon.source.toString() === "") {
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
