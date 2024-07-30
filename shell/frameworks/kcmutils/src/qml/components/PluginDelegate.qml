/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCMUtils

/// @since 6.0, this got renamed from KPluginDelegate to PluginDelegate
LingmoUI.CheckSubtitleDelegate {
    id: listItem

    // Note: when PluginDelegate is embedded in a more complex delegate, model
    // object should be passed down explicitly, but it also means that it may
    // become null right before delegate's destruction.
    required property var model

    property list<T.Action> additionalActions

    property alias leading: leadingProxy.target

    readonly property bool enabledByDefault: model?.enabledByDefault ?? false
    readonly property var metaData: model?.metaData
    readonly property bool configureVisible: model?.config.isValid ?? false

    signal configTriggered()

    // Let Optional chaining (?.) operator fall back to `undefined` which resets the width to an implicit value.
    width: ListView.view?.width

    icon.name: model?.icon ?? ""
    text: model?.name ?? ""
    subtitle: model?.description ?? ""
    checked: model?.enabled ?? false

    // TODO: It should be possible to disable this
    onToggled: model.enabled = checked

    contentItem: RowLayout {
        spacing: LingmoUI.Units.smallSpacing

        // Used by CheckSubtitleDelegate through duck-typing
        readonly property alias truncated: titleSubtitle.truncated

        LayoutItemProxy {
            id: leadingProxy
            visible: target !== null
        }

        LingmoUI.IconTitleSubtitle {
            id: titleSubtitle

            Layout.fillWidth: true
            Layout.maximumWidth: Math.ceil(implicitWidth)

            icon: icon.fromControlsIcon(listItem.icon)
            title: listItem.text
            subtitle: listItem.subtitle
            reserveSpaceForSubtitle: true
        }

        LingmoUI.ActionToolBar {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            alignment: Qt.AlignRight
            actions: [infoAction, configureAction, ...listItem.additionalActions]
        }
    }

    KCMUtils.SettingHighlighter {
        target: listItem.indicator
        highlight: listItem.checked !== listItem.enabledByDefault
    }

    // Take care of displaying the actions
    readonly property LingmoUI.Action __infoAction: LingmoUI.Action {
        id: infoAction

        icon.name: "help-about-symbolic"
        text: i18ndc("kcmutils6", "@info:tooltip", "About")
        displayHint: LingmoUI.DisplayHint.IconOnly
        onTriggered: {
            const aboutDialog = (listItem.ListView.view ?? listItem.parent.ListView.view).__aboutDialog;
            aboutDialog.metaDataInfo = listItem.metaData;
            aboutDialog.open();
        }
    }

    readonly property LingmoUI.Action __configureAction: LingmoUI.Action {
        id: configureAction

        visible: listItem.configureVisible
        enabled: listItem.checked
        icon.name: "configure-symbolic"
        text: i18ndc("kcmutils6", "@info:tooltip", "Configure…")
        displayHint: LingmoUI.DisplayHint.IconOnly
        onTriggered: listItem.configTriggered()
    }
}
