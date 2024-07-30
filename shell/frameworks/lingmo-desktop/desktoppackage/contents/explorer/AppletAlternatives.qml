/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmo.plasmoid
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.lingmo.private.shell 2.0

LingmoCore.Dialog {
    id: dialog
    visualParent: alternativesHelper.applet
    location: alternativesHelper.applet.Plasmoid.location
    hideOnWindowDeactivate: true
    backgroundHints: (alternativesHelper.applet.Plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentPrefersOpaqueBackground) ? LingmoCore.Dialog.SolidBackground : LingmoCore.Dialog.StandardBackground

    Component.onCompleted: {
        flags = flags |  Qt.WindowStaysOnTopHint;
        dialog.show();
    }

    ColumnLayout {
        id: root

        signal configurationChanged

        Layout.minimumWidth: LingmoUI.Units.gridUnit * 20
        Layout.minimumHeight: Math.min(Screen.height - LingmoUI.Units.gridUnit * 10, implicitHeight)

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        property string currentPlugin: ""

        Shortcut {
            sequence: "Escape"
            onActivated: dialog.close()
        }
        Shortcut {
            sequence: "Return"
            onActivated: root.savePluginAndClose()
        }
        Shortcut {
            sequence: "Enter"
            onActivated: root.savePluginAndClose()
        }


        WidgetExplorer {
            id: widgetExplorer
            provides: alternativesHelper.appletProvides
        }

        LingmoExtras.PlasmoidHeading {
            LingmoUI.Heading {
                id: heading
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Alternative Widgets")
                textFormat: Text.PlainText
            }
        }

        // This timer checks with a short delay whether a new item in the list has been hovered by the cursor.
        // If not, then the cursor has left the view and thus no item should be selected.
        Timer {
            id: resetCurrentIndex
            property string oldPlugin
            interval: 100
            onTriggered: {
                if (root.currentPlugin === oldPlugin) {
                    mainList.currentIndex = -1
                    root.currentPlugin = ""
                }
            }
        }

        function savePluginAndClose() {
            alternativesHelper.loadAlternative(currentPlugin);
            dialog.close();
        }

        LingmoComponents3.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.preferredHeight: mainList.contentHeight

            focus: true

            ListView {
                id: mainList

                focus: dialog.visible
                model: widgetExplorer.widgetsModel
                boundsBehavior: Flickable.StopAtBounds
                highlight: LingmoExtras.Highlight {
                    pressed: mainList.currentItem && mainList.currentItem.pressed
                }
                highlightMoveDuration : 0
                highlightResizeDuration: 0

                height: contentHeight+LingmoUI.Units.smallSpacing

                delegate: LingmoComponents3.ItemDelegate {
                    id: listItem

                    implicitHeight: contentLayout.implicitHeight + LingmoUI.Units.smallSpacing * 2
                    width: ListView.view.width

                    onHoveredChanged: {
                        if (hovered) {
                            resetCurrentIndex.stop()
                            mainList.currentIndex = index
                        } else {
                            resetCurrentIndex.oldPlugin = model.pluginName
                            resetCurrentIndex.restart()
                        }
                    }

                    Connections {
                        target: mainList
                        function onCurrentIndexChanged() {
                            if (mainList.currentIndex === index) {
                                root.currentPlugin = model.pluginName
                            }
                        }
                    }

                    onClicked: root.savePluginAndClose()

                    Component.onCompleted: {
                        if (model.pluginName === alternativesHelper.currentPlugin) {
                            root.currentPlugin = model.pluginName
                            setAsCurrent.restart()
                        }
                    }

                    // we don't want to select any entry by default
                    // this cannot be set in Component.onCompleted
                    Timer {
                        id: setAsCurrent
                        interval: 100
                        onTriggered: {
                            mainList.currentIndex = index
                        }
                    }

                    contentItem: RowLayout {
                        id: contentLayout
                        spacing: LingmoUI.Units.largeSpacing

                        LingmoUI.Icon {
                            implicitWidth: LingmoUI.Units.iconSizes.huge
                            implicitHeight: LingmoUI.Units.iconSizes.huge
                            source: model.decoration
                        }

                        ColumnLayout {
                            id: labelLayout

                            readonly property color textColor: listItem.pressed ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor

                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            spacing: 0 // The labels bring their own bottom margins

                            LingmoUI.Heading {
                                level: 4
                                Layout.fillWidth: true
                                text: model.name
                                textFormat: Text.PlainText
                                elide: Text.ElideRight
                                type: model.pluginName === alternativesHelper.currentPlugin ? LingmoExtras.Heading.Type.Primary : LingmoExtras.Heading.Type.Normal
                                color: labelLayout.textColor
                            }

                            LingmoComponents3.Label {
                                Layout.fillWidth: true
                                text: model.description
                                textFormat: Text.PlainText
                                font.pointSize: LingmoUI.Theme.smallFont.pointSize
                                font.family: LingmoUI.Theme.smallFont.family
                                font.bold: model.pluginName === alternativesHelper.currentPlugin
                                opacity: 0.6
                                maximumLineCount: 2
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                                color: labelLayout.textColor
                            }
                        }
                    }
                }
            }
        }
    }
}
