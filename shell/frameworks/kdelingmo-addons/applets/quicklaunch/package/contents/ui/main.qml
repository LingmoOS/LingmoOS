/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2
import QtQuick.Layouts 1.0
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.draganddrop 2.0 as DragAndDrop
import org.kde.lingmo.private.quicklaunch 1.0

import "layout.js" as LayoutManager

PlasmoidItem {
    id: root

    readonly property int maxSectionCount: plasmoid.configuration.maxSectionCount
    readonly property bool showLauncherNames : plasmoid.configuration.showLauncherNames
    readonly property bool enablePopup : plasmoid.configuration.enablePopup
    readonly property string title : plasmoid.formFactor == LingmoCore.Types.Planar ? plasmoid.configuration.title : ""
    readonly property bool vertical : plasmoid.formFactor == LingmoCore.Types.Vertical || (plasmoid.formFactor == LingmoCore.Types.Planar && height > width)
    readonly property bool horizontal : plasmoid.formFactor == LingmoCore.Types.Horizontal
    property bool dragging : false

    Layout.minimumWidth: LayoutManager.minimumWidth()
    Layout.minimumHeight: LayoutManager.minimumHeight()
    Layout.preferredWidth: LayoutManager.preferredWidth()
    Layout.preferredHeight: LayoutManager.preferredHeight()

    preferredRepresentation: fullRepresentation
    Plasmoid.backgroundHints: LingmoCore.Types.DefaultBackground | LingmoCore.Types.ConfigurableBackground

    Item {
        anchors.fill: parent

        DragAndDrop.DropArea {
            anchors.fill: parent
            preventStealing: true
            enabled: !plasmoid.immutable

            onDragEnter: {
                if (event.mimeData.hasUrls) {
                    dragging = true;
                } else {
                    event.ignore();
                }
            }

            onDragMove: {
                var index = grid.indexAt(event.x, event.y);

                if (isInternalDrop(event)) {
                    launcherModel.moveUrl(event.mimeData.source.itemIndex, index);
                } else {
                    launcherModel.showDropMarker(index);
                }

                popup.visible = root.childAt(event.x, event.y) == popupArrow;
            }

            onDragLeave: {
                dragging = false;
                launcherModel.clearDropMarker();
            }

            onDrop: {
                dragging = false;
                launcherModel.clearDropMarker();

                if (isInternalDrop(event)) {
                    event.accept(Qt.IgnoreAction);
                    saveConfiguration();
                } else {
                    var index = grid.indexAt(event.x, event.y);
                    launcherModel.insertUrls(index == -1 ? launcherModel.count : index, event.mimeData.urls);
                    event.accept(event.proposedAction);
                }
            }
        }

        LingmoComponents3.Label {
            id: titleLabel

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            height: LingmoUI.Units.iconSizes.sizeForLabels
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
            elide: Text.ElideMiddle
            text: title
            textFormat: Text.PlainText
        }

        Item {
            id: launcher

            anchors {
                top: title.length ? titleLabel.bottom : parent.top
                left: parent.left
                right: !vertical && popupArrow.visible ? popupArrow.left : parent.right
                bottom: vertical && popupArrow.visible ? popupArrow.top : parent.bottom
            }

            GridView {
                id: grid
                anchors.fill: parent
                interactive: false
                flow: horizontal ? GridView.FlowTopToBottom : GridView.FlowLeftToRight
                cellWidth: LayoutManager.preferredCellWidth()
                cellHeight: LayoutManager.preferredCellHeight()
                visible: count

                model: UrlModel {
                    id: launcherModel
                }

                delegate: IconItem { }

                function moveItemToPopup(iconItem, url) {
                    if (!popupArrow.visible) {
                        return;
                    }

                    popup.visible = true;
                    popup.mainItem.popupModel.insertUrl(popup.mainItem.popupModel.count, url);
                    popup.mainItem.listView.currentIndex = popup.mainItem.popupModel.count - 1;
                    iconItem.removeLauncher();
                }
            }

            LingmoUI.Icon {
                id: defaultIcon
                anchors.fill: parent
                source: "fork"
                visible: !grid.visible

                LingmoCore.ToolTipArea {
                    anchors.fill: parent
                    mainText: i18n("Quicklaunch")
                    subText: i18nc("@info", "Add launchers by Drag and Drop or by using the context menu.")
                    location: Plasmoid.location
                }
            }
        }

        LingmoCore.Dialog {
            id: popup
            type: LingmoCore.Dialog.PopupMenu
            flags: Qt.WindowStaysOnTopHint
            hideOnWindowDeactivate: true
            location: plasmoid.location
            visualParent: vertical ? popupArrow : root

            mainItem: Popup {
                Keys.onEscapePressed: popup.visible = false
            }
        }

        LingmoCore.ToolTipArea {
            id: popupArrow
            visible: enablePopup
            location: Plasmoid.location

            anchors {
                top: vertical ? undefined : parent.top
                right: parent.right
                bottom: parent.bottom
            }

            subText: popup.visible ? i18n("Hide icons") : i18n("Show hidden icons")

            MouseArea {
                id: arrowMouseArea
                anchors.fill: parent

                activeFocusOnTab: parent.visible

                Keys.onPressed: {
                    switch (event.key) {
                    case Qt.Key_Space:
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Select:
                        arrowMouseArea.clicked(null);
                        break;
                    }
                }
                Accessible.name: parent.subText
                Accessible.role: Accessible.Button

                onClicked: {
                    popup.visible = !popup.visible
                }

                LingmoUI.Icon {
                    anchors.fill: parent

                    rotation: popup.visible ? 180 : 0
                    Behavior on rotation {
                        RotationAnimation {
                            duration: LingmoUI.Units.shortDuration * 3
                        }
                    }

                    source: {
                        if (plasmoid.location == LingmoCore.Types.TopEdge) {
                            return "arrow-down";
                        } else if (plasmoid.location == LingmoCore.Types.LeftEdge) {
                            return "arrow-right";
                        } else if (plasmoid.location == LingmoCore.Types.RightEdge) {
                            return "arrow-left";
                        } else if (vertical) {
                            return "arrow-right";
                        } else {
                            return "arrow-up";
                        }
                    }
                }
            }
        }
    }

    Logic {
        id: logic

        onLauncherAdded: {
            var m = isPopup ? popup.mainItem.popupModel : launcherModel;
            m.appendUrl(url);
        }

        onLauncherEdited: {
            var m = isPopup ? popup.mainItem.popupModel : launcherModel;
            m.changeUrl(index, url);
        }
    }

    // States to fix binding loop with enabled popup
    states: [
        State {
            name: "normal"
            when: !vertical

            PropertyChanges {
                target: popupArrow
                width: LingmoUI.Units.iconSizes.smallMedium
                height: root.height
            }
        },

        State {
            name: "vertical"
            when: vertical

            PropertyChanges {
                target: popupArrow
                width: root.width
                height: LingmoUI.Units.iconSizes.smallMedium
            }
        }
    ]

    Connections {
        target: plasmoid.configuration
       function onLauncherUrlsChanged() {
            launcherModel.urlsChanged.disconnect(saveConfiguration);
            launcherModel.setUrls(plasmoid.configuration.launcherUrls);
            launcherModel.urlsChanged.connect(saveConfiguration);
        }
    }

    Plasmoid.contextualActions: [
        LingmoCore.Action {
            text: i18nc("@action", "Add Launcher…")
            icon.name: "list-add"
            onTriggered: logic.addLauncher()
        }
    ]

    Component.onCompleted: {
        launcherModel.setUrls(plasmoid.configuration.launcherUrls);
        launcherModel.urlsChanged.connect(saveConfiguration);
    }

    function saveConfiguration()
    {
        if (!dragging) {
            plasmoid.configuration.launcherUrls = launcherModel.urls();
        }
    }

    function isInternalDrop(event)
    {
        return event.mimeData.source
            && event.mimeData.source.GridView
            && event.mimeData.source.GridView.view == grid;
    }
}
