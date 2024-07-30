/*
 *   SPDX-FileCopyrightText: 2021 Jan Blackquill <uhhadd@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

ItemDelegate {
    id: delegate

    property bool showArrow: false
    property bool selected: delegate.highlighted || delegate.pressed
    property bool isSearching: false
    property real leadingPadding: 0
    required property bool showDefaultIndicator

    width: ListView.view?.width ?? 0

    Accessible.name: text
    Accessible.onPressAction: clicked()

    contentItem: RowLayout {
        spacing: LingmoUI.Units.smallSpacing

        LingmoUI.IconTitleSubtitle {
            Layout.fillWidth: true
            Layout.leftMargin: delegate.leadingPadding
            icon: icon.fromControlsIcon(delegate.icon)
            title: delegate.text
            selected: delegate.selected
        }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: LingmoUI.Units.largeSpacing
            Layout.preferredHeight: LingmoUI.Units.largeSpacing

            radius: width * 0.5
            visible: delegate.showDefaultIndicator && systemsettings.defaultsIndicatorsVisible
            LingmoUI.Theme.colorSet: LingmoUI.Theme.View
            color: LingmoUI.Theme.neutralTextColor
        }

        LingmoUI.Icon {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: LingmoUI.Units.iconSizes.small
            Layout.preferredHeight: LingmoUI.Units.iconSizes.small

            opacity: delegate.showArrow ? 0.7 : 0.0
            source: LayoutMirroring.enabled ? "go-next-symbolic-rtl" : "go-next-symbolic"
            selected: delegate.selected
        }
    }
}
