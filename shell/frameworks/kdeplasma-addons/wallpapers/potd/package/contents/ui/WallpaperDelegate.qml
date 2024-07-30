/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import Qt5Compat.GraphicalEffects as GE
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami as Kirigami

FocusScope {
    // FocusScope can pass Tab to inline buttons
    id: delegate

    readonly property int shadowOffset: thumbnail.shadow.size - thumbnail.shadow.yOffset
    readonly property bool isNull: wallpaperImage.status !== Image.Ready

    /**
     * The background color of the preview area when the image is loaded
     */
    property color backgroundColor

    /**
     * The local path of the wallpaper
     */
    property url localUrl

    /**
     * The website of the wallpaper
     */
    property url infoUrl

    /**
     * The title of the wallpaper
     */
    property string title

    /**
     * The author of the wallpaper
     */
    property string author

    /**
     * Set it to true when a thumbnail is actually available: when false,
     * only an icon ("edit-none") will be shown instead of the actual thumbnail.
     */
    property bool thumbnailAvailable: false

    /**
     * Set it to true when a thumbnail is still being loaded: when false,
     * the BusyIndicator will be shown.
     */
    property bool thumbnailLoading: false

    /**
     * A list of extra actions for the thumbnails. They will be shown as
     * icons on the bottom-right corner of the thumbnail on mouse over
     */
    property list<QtObject> actions

    ActionContextMenu {
        id: contextMenu
        showAllActions: thumbnailAvailable
        actions: delegate.actions
    }

    Keys.onMenuPressed: contextMenu.popup(delegate, thumbnail.x, thumbnail.y + thumbnail.height)
    Keys.onSpacePressed: contextMenu.popup(delegate, thumbnail.x, thumbnail.y + thumbnail.height)

    onThumbnailLoadingChanged: {
        if (!thumbnailLoading) {
            if (wallpaperImage.source === Qt.resolvedUrl(delegate.localUrl)) {
                wallpaperImage.source = "";
            }
            wallpaperImage.source = delegate.localUrl;
        } else {
            wallpaperImage.source = delegate.localUrl;
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: contextMenu.popup()
    }

    TapHandler {
        id: openUrlTapHandler
        enabled: hoverHandler.enabled
        acceptedButtons: Qt.LeftButton
        onTapped: Qt.openUrlExternally(delegate.infoUrl)
    }

    HoverHandler {
        id: hoverHandler
        enabled: delegate.thumbnailAvailable && delegate.infoUrl.toString().length > 0
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }

    QQC2.ToolTip {
        text: delegate.infoUrl.toString()
        visible: hoverHandler.enabled && !contextMenu.opened
                 && (hoverHandler.hovered
                     || thumbnailArea.activeFocus
                     || (Kirigami.Settings.isMobile && openUrlTapHandler.pressed))
    }

    // From kdeclarative/src/qmlcontrols/kcmcontrols/qml/GridDelegate.qml
    Kirigami.ShadowedRectangle {
        id: thumbnail
        anchors.fill: parent
        radius: Kirigami.Units.cornerRadius
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        shadow.xOffset: 0
        shadow.yOffset: 2
        shadow.size: 10
        shadow.color: Qt.rgba(0, 0, 0, 0.3)

        color: thumbnailArea.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.backgroundColor

        Rectangle {
            id: thumbnailArea
            radius: Math.round(Kirigami.Units.cornerRadius / 2)
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
            }

            color: !delegate.thumbnailAvailable || delegate.thumbnailLoading ? Kirigami.Theme.backgroundColor : delegate.backgroundColor

            activeFocusOnTab: true
            Accessible.name: delegate.thumbnailAvailable ? i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis", "Today's picture")
                           : delegate.thumbnailLoading ? i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis", "Loading")
                                                       : i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis", "Unavailable")
            Accessible.description: delegate.thumbnailAvailable ? i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis for an image %1 title %2 author", "%1 Author: %2. Right-click on the image to see more actions.", delegate.title, delegate.author)
                                  : delegate.thumbnailLoading ? i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis", "The wallpaper is being fetched from the Internet.")
                                                              : i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis", "Failed to fetch the wallpaper from the Internet.")

            Image {
                id: wallpaperImage

                anchors.fill: parent
                autoTransform: false
                cache: false
                fillMode: cfg_FillMode || Image.PreserveAspectCrop
                smooth: true

                Drag.dragType: Drag.Automatic
                Drag.supportedActions: Qt.CopyAction
                Drag.mimeData: {
                    "text/uri-list" : [delegate.localUrl],
                    "text/plain": delegate.title,
                }

                DragHandler {
                    id: dragHandler

                    onActiveChanged: if (active) {
                        parent.grabToImage((result) => {
                            parent.Drag.imageSource = result.url;
                            parent.Drag.active = dragHandler.active;
                        });
                    } else {
                        parent.Drag.active = false;
                        parent.Drag.imageSource = "";
                    }
                }

                // CachedProvider will load the image from cache, but we would like to show the real loading status.
                layer.enabled: delegate.thumbnailLoading
                layer.effect: GE.HueSaturation {
                    cached: true

                    lightness: 0.5
                    saturation: 0.9

                    layer.enabled: true
                    layer.effect: GE.GaussianBlur {
                        cached: true

                        radius: 128
                        deviation: 12
                        samples: 63

                        transparentBorder: false
                    }
                }
            }

            Loader {
                active: delegate.thumbnailLoading || !delegate.thumbnailAvailable

                anchors.centerIn: parent
                opacity: 0.5
                visible: active

                width: Kirigami.Units.iconSizes.large
                height: width

                sourceComponent: delegate.thumbnailLoading ? busyIndicator : fallbackIcon

                Component {
                    id: busyIndicator

                    QQC2.BusyIndicator {
                        anchors.fill: parent
                    }
                }

                // "None/There's nothing here" indicator
                Component {
                    id: fallbackIcon

                    Kirigami.Icon {
                        anchors.fill: parent
                        source: "edit-none"
                    }
                }
            }

            RowLayout {
                anchors {
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                    bottom: parent.bottom
                    bottomMargin: Kirigami.Units.smallSpacing
                }

                // Always show above thumbnail content
                z: 9999

                Repeater {
                    model: delegate.actions
                    delegate: QQC2.Button {
                        icon.name: modelData.icon.name
                        activeFocusOnTab: visible
                        onClicked: modelData.trigger()
                        enabled: modelData.enabled
                        visible: modelData.visible

                        Accessible.name: modelData.tooltip
                        Accessible.description: modelData.Accessible.description

                        QQC2.ToolTip {
                            visible: modelData.tooltip.length > 0
                                     && ((Kirigami.Settings.isMobile ? parent.pressed : parent.hovered)
                                          || (parent.activeFocus && (parent.focusReason === Qt.TabFocusReason || parent.focusReason === Qt.BacktabFocusReason)))
                            text: modelData.tooltip
                        }
                    }
                }
            }
        }
    }
}
