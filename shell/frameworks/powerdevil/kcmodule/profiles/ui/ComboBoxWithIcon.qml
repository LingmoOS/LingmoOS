/*
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels

QQC2.ComboBox {
    id: comboBox

    required model

    // HACK QQC2 doesn't support icons, so we just tamper with the desktop style ComboBox's background
    function loadProps() {
        if (!background || !background.hasOwnProperty("properties")) {
            // not a KQuickStyleItem
            return;
        }

        const props = background.properties || {};

        background.properties = Qt.binding(function() {
            const modelIndex = model.index(currentIndex, 0);
            props.currentIcon = model.data(modelIndex, model.KItemModels.KRoleNames.role("iconName"));
            props.iconColor = Kirigami.Theme.textColor;
            return props;
        });
    }

    Component.onCompleted: { loadProps(); }
    onCurrentIndexChanged: { loadProps(); }

    delegate: QQC2.ItemDelegate {
        required property string index

        // model roles from roleNames(), expose these in your QAbstractItemModel if you haven't already
        required property string name
        required property string iconName

        width: comboBox.popup.width
        text: name
        highlighted: index === currentIndex
        icon.name: iconName
    }
}
