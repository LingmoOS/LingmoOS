/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 6.2
import org.kde.prison.scanner 1.0 as Prison

ApplicationWindow {
    width: 1024
    height: 768
    visible: true

    VideoOutput {
        id: viewFinder
        anchors.fill: parent
    }

    Prison.VideoScanner {
        id: scanner
        formats: Prison.Format.QRCode | Prison.Format.Aztec
        onResultChanged: console.log(result.text, result.format);
        videoSink: viewFinder.videoSink
    }

    CaptureSession {
        camera: Camera {
            id: camera
            active: true
            function onErrorOccurred(error, errorString) {
                console.log("Camera error: " + camera.errorString)
            }
        }
        videoOutput: viewFinder
    }

    Rectangle {
        color: "#80ff0000"
        x: viewFinder.contentRect.x + scanner.result.boundingRect.x / viewFinder.sourceRect.width * viewFinder.contentRect.width
        y: viewFinder.contentRect.y + scanner.result.boundingRect.y / viewFinder.sourceRect.height * viewFinder.contentRect.height
        width: scanner.result.boundingRect.width / viewFinder.sourceRect.width * viewFinder.contentRect.width
        height: scanner.result.boundingRect.height / viewFinder.sourceRect.height * viewFinder.contentRect.height
    }
}

