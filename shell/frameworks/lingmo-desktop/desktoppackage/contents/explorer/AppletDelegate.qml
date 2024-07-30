/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.core as LingmoCore
import org.kde.kwindowsystem
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.graphicaleffects as KGraphicalEffects

Item {
    id: delegate

    readonly property string pluginName: model.pluginName
    readonly property bool pendingUninstall: pendingUninstallTimer.applets.indexOf(pluginName) > -1
    readonly property bool pressed: tapHandler.pressed

    width: list.cellWidth
    height: list.cellHeight

    HoverHandler {
        id: hoverHandler
        onHoveredChanged: if (hovered) delegate.GridView.view.currentIndex = index
    }

    TapHandler {
        id: tapHandler
        enabled: !delegate.pendingUninstall && model.isSupported
        onTapped: widgetExplorer.addApplet(delegate.pluginName)
    }

    LingmoCore.ToolTipArea {
        anchors.fill: parent
        visible: !model.isSupported
        mainText: i18n("Unsupported Widget")
        subText: model.unsupportedMessage
    }

    // Avoid repositioning delegate item after dragFinished
    Item {
        anchors.fill: parent
        enabled: model.isSupported

        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.MoveAction | Qt.LinkAction
        Drag.mimeData: {
            "text/x-plasmoidservicename" : delegate.pluginName,
        }
        Drag.onDragStarted: {
            KWindowSystem.showingDesktop = true;
            main.draggingWidget = true;
        }
        Drag.onDragFinished: {
            main.draggingWidget = false;
        }

        DragHandler {
            id: dragHandler
            enabled: !delegate.pendingUninstall && model.isSupported

            onActiveChanged: if (active) {
                iconContainer.grabToImage(function(result) {
                    if (!dragHandler.active) {
                        return;
                    }
                    parent.Drag.imageSource = result.url;
                    parent.Drag.active = dragHandler.active;
                }, Qt.size(LingmoUI.Units.iconSizes.huge, LingmoUI.Units.iconSizes.huge));
            } else {
                parent.Drag.active = false;
                parent.Drag.imageSource = "";
            }
        }
    }

    ColumnLayout {
        id: mainLayout

        readonly property color textColor: tapHandler.pressed ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor

        spacing: LingmoUI.Units.smallSpacing
        anchors {
            left: parent.left
            right: parent.right
            //bottom: parent.bottom
            margins: LingmoUI.Units.smallSpacing * 2
            rightMargin: LingmoUI.Units.smallSpacing * 2 // don't cram the text to the border too much
            top: parent.top
        }

        Item {
            id: iconContainer
            width: LingmoUI.Units.iconSizes.enormous
            height: width
            Layout.alignment: Qt.AlignHCenter
            opacity: delegate.pendingUninstall ? 0.6 : 1
            Behavior on opacity {
                OpacityAnimator {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            Item {
                id: iconWidget
                anchors.fill: parent
                LingmoUI.Icon {
                    anchors.fill: parent
                    source: model.decoration
                    visible: model.screenshot === ""
                    selected: tapHandler.pressed
                    enabled: model.isSupported
                }
                Image {
                    width: LingmoUI.Units.iconSizes.enormous
                    height: width
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: model.screenshot
                }
            }

            Item {
                id: badgeMask
                anchors.fill: parent

                Rectangle {
                    x: Math.round(-LingmoUI.Units.smallSpacing * 1.5 / 2)
                    y: x
                    width: runningBadge.width + Math.round(LingmoUI.Units.smallSpacing * 1.5)
                    height: width
                    radius: height
                    visible: running && delegate.GridView.isCurrentItem
                }
            }

            KGraphicalEffects.BadgeEffect {
                anchors.fill: parent
                source: ShaderEffectSource {
                    sourceItem: iconWidget
                    hideSource: true
                    live: false
                }
                mask: ShaderEffectSource {
                    id: maskShaderSource
                    sourceItem: badgeMask
                    hideSource: true
                    live: false
                }
            }

            Rectangle {
                id: runningBadge
                width: height
                height: Math.round(LingmoUI.Units.iconSizes.sizeForLabels * 1.3)
                radius: height
                color: LingmoUI.Theme.highlightColor
                visible: running && delegate.GridView.isCurrentItem
                onVisibleChanged: maskShaderSource.scheduleUpdate()

                LingmoComponents.Label {
                    id: countLabel
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: running
                    textFormat: Text.PlainText
                }
            }


            LingmoComponents.ToolButton {
                id: uninstallButton
                anchors {
                    top: parent.top
                    right: parent.right
                }
                icon.name: delegate.pendingUninstall ? "edit-undo" : "edit-delete"
                // we don't really "undo" anything but we'll pretend to the user that we do
                LingmoComponents.ToolTip.delay: LingmoUI.Units.toolTipDelay
                LingmoComponents.ToolTip.visible: hovered
                LingmoComponents.ToolTip.text: delegate.pendingUninstall ? i18nd("lingmo_shell_org.kde.lingmo.desktop", "Undo uninstall")
                                                    : i18nd("lingmo_shell_org.kde.lingmo.desktop", "Uninstall widget")
                flat: false
                visible: model.local && delegate.GridView.isCurrentItem && !dragHandler.active

                onHoveredChanged: {
                    if (hovered) {
                        // hovering the uninstall button triggers onExited of the main mousearea
                        delegate.GridView.view.currentIndex = index
                    }
                }

                onClicked: {
                    var pending = pendingUninstallTimer.applets
                    if (delegate.pendingUninstall) {
                        var index = pending.indexOf(pluginName)
                        if (index > -1) {
                            pending.splice(index, 1)
                        }
                    } else {
                        pending.push(pluginName)
                    }
                    pendingUninstallTimer.applets = pending

                    if (pending.length) {
                        pendingUninstallTimer.restart()
                    } else {
                        pendingUninstallTimer.stop()
                    }
                }
            }
        }
        LingmoUI.Heading {
            id: heading
            Layout.fillWidth: true
            level: 4
            text: model.name
            textFormat: Text.PlainText
            elide: Text.ElideRight
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            lineHeight: 0.95
            horizontalAlignment: Text.AlignHCenter
            color: mainLayout.textColor
        }
        LingmoComponents.Label {
            Layout.fillWidth: true
            // otherwise causes binding loop due to the way the Lingmo sets the height
            height: implicitHeight
            text: model.description
            textFormat: Text.PlainText
            font: LingmoUI.Theme.smallFont
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            maximumLineCount: heading.lineCount === 1 ? 3 : 2
            horizontalAlignment: Text.AlignHCenter
            color: mainLayout.textColor
        }
    }
}
