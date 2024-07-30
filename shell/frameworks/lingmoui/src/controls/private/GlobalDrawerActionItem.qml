/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Controls.impl as QQC2Impl
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

QQC2.ItemDelegate {
    id: listItem

    required property T.Action tAction
    // `as` case operator is still buggy
    readonly property LingmoUI.Action kAction: tAction instanceof LingmoUI.Action ? tAction : null

    readonly property bool actionVisible: kAction?.visible ?? true
    readonly property bool isSeparator: kAction?.separator ?? false
    readonly property bool isExpandable: kAction?.expandible ?? false
    readonly property bool hasChildren: kAction ? kAction.children.length > 0 : false
    readonly property bool hasVisibleMenu: actionsMenu?.visible ?? false
    readonly property bool hasToolTip: kAction ? kAction.tooltip !== "" : false

    checked: checkedBinding()
    highlighted: checked
    activeFocusOnTab: true

    width: parent.width

    contentItem: RowLayout {
        spacing: LingmoUI.Units.largeSpacing

        LingmoUI.Icon {
            id: iconItem
            color: listItem.tAction.icon.color
            source: listItem.tAction.icon.name || listItem.tAction.icon.source

            readonly property int size: LingmoUI.Units.iconSizes.smallMedium
            Layout.minimumHeight: size
            Layout.maximumHeight: size
            Layout.minimumWidth: size
            Layout.maximumWidth: size

            selected: (listItem.highlighted || listItem.checked || listItem.down)
            visible: source !== undefined && !listItem.isSeparator
        }

        QQC2Impl.MnemonicLabel {
            id: labelItem
            visible: !listItem.isSeparator
            text: width > height * 2 ? listItem.LingmoUI.MnemonicData.mnemonicLabel : ""

            // Work around Qt bug where left aligned text is not right aligned
            // in RTL mode unless horizontalAlignment is explicitly set.
            // https://bugreports.qt.io/browse/QTBUG-95873
            horizontalAlignment: Text.AlignLeft

            Layout.fillWidth: true
            mnemonicVisible: listItem.LingmoUI.MnemonicData.active
            color: (listItem.highlighted || listItem.checked || listItem.down) ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
            elide: Text.ElideRight
            font: listItem.font
            opacity: {
                if (root.collapsed) {
                    return 0;
                } else if (!listItem.enabled) {
                    return 0.6;
                } else {
                    return 1.0;
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration/2
                    easing.type: Easing.InOutQuad
                }
            }
        }

        LingmoUI.Separator {
            id: separatorAction

            visible: listItem.isSeparator
            Layout.fillWidth: true
        }

        LingmoUI.Icon {
            isMask: true
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: !root.collapsed ? 0 : -width
            Layout.preferredHeight: !root.collapsed ? LingmoUI.Units.iconSizes.small : LingmoUI.Units.iconSizes.small/2
            opacity: 0.7
            selected: listItem.checked || listItem.down
            Layout.preferredWidth: Layout.preferredHeight
            source: listItem.mirrored ? "go-next-symbolic-rtl" : "go-next-symbolic"
            visible: (!listItem.isExpandable || root.collapsed) && !listItem.isSeparator && listItem.hasChildren
        }
    }

    Accessible.name: listItem.tAction?.text ?? ""
    LingmoUI.MnemonicData.enabled: enabled && visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.MenuItem
    LingmoUI.MnemonicData.label: tAction?.text ?? ""

    Shortcut {
        sequence: listItem.LingmoUI.MnemonicData.sequence
        onActivated: listItem.clicked()
    }

    property ActionsMenu actionsMenu: ActionsMenu {
        x: Qt.application.layoutDirection === Qt.RightToLeft ? -width : listItem.width
        actions: listItem.kAction?.children ?? []
        submenuComponent: ActionsMenu {}

        onVisibleChanged: {
            if (visible) {
                stackView.openSubMenu = listItem.actionsMenu;
            } else if (stackView.openSubMenu === listItem.actionsMenu) {
                stackView.openSubMenu = null;
            }
        }
    }

    // TODO: animate the hide by collapse
    visible: actionVisible && opacity > 0
    opacity: !root.collapsed || iconItem.source.toString().length > 0

    Behavior on opacity {
        NumberAnimation {
            duration: LingmoUI.Units.longDuration / 2
            easing.type: Easing.InOutQuad
        }
    }

    enabled: tAction?.enabled ?? false

    hoverEnabled: (!isExpandable || root.collapsed) && !LingmoUI.Settings.tabletMode && !isSeparator
    font.pointSize: isExpandable ? LingmoUI.Theme.defaultFont.pointSize * 1.30 : LingmoUI.Theme.defaultFont.pointSize
    height: implicitHeight * opacity

    QQC2.ToolTip {
        visible: !listItem.isSeparator
            && (listItem.hasToolTip || root.collapsed)
            && !listItem.hasVisibleMenu
            && listItem.hovered
            && text.length > 0

        text: listItem.kAction?.tooltip ?? listItem.tAction?.text ?? ""
        delay: LingmoUI.Units.toolTipDelay
        y: (listItem.height - height) / 2
        x: Qt.application.layoutDirection === Qt.RightToLeft ? -width : listItem.width
    }

    onHoveredChanged: {
        if (!hovered) {
            return;
        }
        if (stackView.openSubMenu) {
            stackView.openSubMenu.visible = false;

            if (actionsMenu.count > 0) {
                actionsMenu.popup(this, width, 0);
            }
        }
    }

    onClicked: trigger()
    Accessible.onPressAction: trigger()
    Keys.onEnterPressed: event => trigger()
    Keys.onReturnPressed: event => trigger()

    function trigger() {
        tAction?.trigger();

        if (hasChildren) {
            if (root.collapsed) {
                if (actionsMenu.count > 0 && !actionsMenu.visible) {
                    stackView.openSubMenu = actionsMenu;
                    actionsMenu.popup(this, width, 0);
                }
            } else {
                stackView.push(menuComponent, {
                    model: kAction?.children ?? [],
                    level: level + 1,
                    current: tAction,
                });
            }
        } else if (root.resetMenuOnTriggered) {
            root.resetMenu();
        }
        checked = Qt.binding(() => checkedBinding());
    }

    function checkedBinding(): bool {
        return (tAction?.checked || actionsMenu?.visible) ?? false;
    }

    Keys.onDownPressed: event => nextItemInFocusChain().focus = true
    Keys.onUpPressed: event => nextItemInFocusChain(false).focus = true
}
