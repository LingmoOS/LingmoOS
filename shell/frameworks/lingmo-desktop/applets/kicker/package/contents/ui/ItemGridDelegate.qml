/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.core as LingmoCore

import "code/tools.js" as Tools

Item {
    id: item

    width: GridView.view.cellWidth
    height: width

    enabled: !model.disabled

    property bool showLabel: true

    property int itemIndex: model.index
    property string favoriteId: model.favoriteId !== undefined ? model.favoriteId : ""
    property url url: model.url !== undefined ? model.url : ""
    property var icon: model.decoration !== undefined ? model.decoration : ""
    property var m: model
    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList === true)))

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.display ?? ""

    function openActionMenu(x, y) {
        var actionList = hasActionList ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, GridView.view.model.favoritesModel, model.favoriteId);
        actionMenu.visualParent = item;
        actionMenu.open(x, y);
    }

    function actionTriggered(actionId, actionArgument) {
        var close = (Tools.triggerAction(GridView.view.model, model.index, actionId, actionArgument) === true);

        if (close) {
            root.toggle();
        }
    }

    LingmoUI.Icon {
        id: icon

        y: item.showLabel ? (2 * highlightItemSvg.margins.top) : undefined

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: item.showLabel ? undefined : parent.verticalCenter

        width: iconSize
        height: width

        animated: false

        source: model.decoration
    }

    LingmoComponents3.Label {
        id: label

        visible: item.showLabel

        anchors {
            top: icon.bottom
            topMargin: LingmoUI.Units.smallSpacing
            left: parent.left
            leftMargin: highlightItemSvg.margins.left
            right: parent.right
            rightMargin: highlightItemSvg.margins.right
        }

        horizontalAlignment: Text.AlignHCenter

        maximumLineCount: 2
        elide: Text.ElideMiddle
        wrapMode: Text.Wrap

        color: "white" // FIXME TODO: Respect theming?

        text: model?.name ?? model.display ?? ""
        textFormat: Text.PlainText
    }

    LingmoCore.ToolTipArea {
        id: toolTip

        mainText: model.display ?? ""

        anchors.fill: parent
        active: root.visible && mainText !== "" && label.truncated
        mainItem: toolTipDelegate

        onContainsMouseChanged: item.GridView.view.itemContainsMouseChanged(containsMouse)
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Menu && hasActionList) {
            event.accepted = true;
            openActionMenu(item);
        } else if ((event.key === Qt.Key_Enter || event.key === Qt.Key_Return)) {
            event.accepted = true;

            if ("trigger" in GridView.view.model) {
                GridView.view.model.trigger(index, "", null);
                root.toggle();
            }

            itemGrid.itemActivated(index, "", null);
        }
    }
}
