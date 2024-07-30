/*
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Window 2.15

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.wallpapers.potd 1.0
import org.kde.plasma.plasmoid 2.0

WallpaperItem {
    id: root

    contextualActions: [
        PlasmaCore.Action {
            text: i18nd("plasma_wallpaper_org.kde.potd", "Open Wallpaper Image")
            icon.name: "document-open"
            onTriggered: Qt.openUrlExternally(backend.localUrl);
        }
    ]

    QQC2.StackView {
        id: imageView
        anchors.fill: parent

        readonly property int fillMode: root.configuration.FillMode
        readonly property size sourceSize: Qt.size(imageView.width * Screen.devicePixelRatio, imageView.height * Screen.devicePixelRatio)
        property Item pendingImage
        property bool doesSkipAnimation: true

        onFillModeChanged: Qt.callLater(imageView.loadImage)
        onSourceSizeChanged: Qt.callLater(imageView.loadImage)

        function loadImage() {
            if (backend.localUrl.length === 0) {
                return;
            }
            if (imageView.pendingImage) {
                imageView.pendingImage.statusChanged.disconnect(replaceWhenLoaded);
                imageView.pendingImage.destroy();
                imageView.pendingImage = null;
            }

            imageView.doesSkipAnimation = imageView.empty || sourceSize !== imageView.currentItem.sourceSize;
            imageView.pendingImage = imageComponent.createObject(imageView, {
                "source": backend.localUrl,
                "fillMode": imageView.fillMode,
                "opacity": imageView.doesSkipAnimation ? 1 : 0,
                "sourceSize": imageView.sourceSize,
                "width": imageView.width,
                "height": imageView.height,
            });
            imageView.pendingImage.statusChanged.connect(imageView.replaceWhenLoaded);
            imageView.replaceWhenLoaded();
        }

        function replaceWhenLoaded() {
            if (imageView.pendingImage.status === Image.Loading) {
                return;
            }
            imageView.pendingImage.statusChanged.disconnect(imageView.replaceWhenLoaded);
            imageView.replace(imageView.pendingImage, {}, imageView.doesSkipAnimation ? QQC2.StackView.Immediate : QQC2.StackView.Transition);
            imageView.pendingImage = null;
        }

        PotdBackend {
            id: backend
            identifier: root.configuration.Provider
            arguments: {
                if (identifier === "bing") {
                    // Bing supports 1366/1920/UHD resolutions
                    const w = imageView.sourceSize.width > 1920 ? 3840 : 1920;
                    const h = imageView.sourceSize.height > 1080 ? 2160 : 1080;
                    return [w, h];
                }
                return [];
            }
            updateOverMeteredConnection: root.configuration.UpdateOverMeteredConnection

            onImageChanged: Qt.callLater(imageView.loadImage)
            onLocalUrlChanged: Qt.callLater(imageView.loadImage)
        }

        Component {
            id: imageComponent

            Image {
                asynchronous: true
                cache: false
                autoTransform: true
                smooth: true

                QQC2.StackView.onActivated: root.accentColorChanged()
                QQC2.StackView.onDeactivated: destroy()
                QQC2.StackView.onRemoved: destroy()
            }
        }

        Rectangle {
            id: backgroundColor
            anchors.fill: parent
            color: root.configuration.Color
            Behavior on color {
                ColorAnimation { duration: Kirigami.Units.longDuration }
            }
        }

        replaceEnter: Transition {
            OpacityAnimator {
                id: replaceEnterOpacityAnimator
                to: 1
                // As the wallpaper is updated once a day, the transition should be longer.
                duration: Math.round(Kirigami.Units.veryLongDuration * 5)
            }
        }
        // Keep the old image around till the new one is fully faded in
        // If we fade both at the same time you can see the background behind glimpse through
        replaceExit: Transition {
            PauseAnimation {
                // 500: The exit transition starts first and can be completed earlier than the enter transition
                duration: replaceEnterOpacityAnimator.duration + 500
            }
        }
    }
}
