/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.kwin 3.0 as KWin

KWin.TabBoxSwitcher {
    id: tabBox

    readonly property real screenFactor: screenGeometry.width / screenGeometry.height

    currentIndex: thumbnailListView.currentIndex

    LingmoCore.Dialog {
        id: dialog
        location: Qt.application.layoutDirection === Qt.RightToLeft ? LingmoCore.Types.RightEdge : LingmoCore.Types.LeftEdge
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: screenGeometry.x + (Qt.application.layoutDirection === Qt.RightToLeft ? screenGeometry.width - width : 0)
        y: screenGeometry.y

        mainItem: LingmoComponents.ScrollView {
            id: dialogMainItem

            focus: true

            contentWidth: tabBox.screenGeometry.width * 0.15
            height: tabBox.screenGeometry.height - dialog.margins.top - dialog.margins.bottom

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            ListView {
                id: thumbnailListView
                focus: true
                model: tabBox.model
                spacing: LingmoUI.Units.smallSpacing
                highlightMoveDuration: LingmoUI.Units.longDuration
                highlightResizeDuration: 0

                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {
                        thumbnailListView.currentIndex = tabBox.currentIndex;
                        thumbnailListView.positionViewAtIndex(thumbnailListView.currentIndex, ListView.Contain)
                    }
                }

                delegate: MouseArea {
                    width: thumbnailListView.width
                    height: delegateColumn.height + 2 * delegateColumn.y
                    focus: ListView.isCurrentItem

                    Accessible.name: model.caption
                    Accessible.role: Accessible.ListItem

                    onClicked: {
                        if (tabBox.noModifierGrab) {
                            tabBox.model.activate(index);
                        } else {
                            thumbnailListView.currentIndex = index;
                        }
                    }

                    ColumnLayout {
                        id: delegateColumn
                        anchors.horizontalCenter: parent.horizontalCenter
                        // anchors.centerIn causes layouting glitches
                        y: LingmoUI.Units.smallSpacing
                        width: parent.width - 2 * LingmoUI.Units.smallSpacing
                        spacing: LingmoUI.Units.smallSpacing

                        Item {
                            Layout.fillWidth: true
                            implicitHeight: Math.round(delegateColumn.width / tabBox.screenFactor)

                            KWin.WindowThumbnail {
                                anchors.fill: parent
                                wId: windowId
                            }
                        }

                        RowLayout {
                            spacing: LingmoUI.Units.smallSpacing
                            Layout.fillWidth: true

                            LingmoUI.Icon {
                                Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
                                Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
                                source: model.icon
                            }

                            LingmoUI.Heading {
                                Layout.fillWidth: true
                                height: undefined
                                level: 4
                                text: model.caption
                                elide: Text.ElideRight
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                maximumLineCount: 2
                                lineHeight: 0.95
                                textFormat: Text.PlainText
                            }
                        }
                    }
                }

                highlight: LingmoExtras.Highlight {}

                LingmoUI.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - LingmoUI.Units.largeSpacing * 2
                    icon.source: "edit-none"
                    text: i18ndc("kwin", "@info:placeholder no entries in the task switcher", "No open windows")
                    visible: thumbnailListView.count === 0
                }
            }
        }
    }
}

