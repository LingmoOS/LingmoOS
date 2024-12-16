// Copyright (C) 2023 pengwenhao <pengwenhao@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import Wallpaper

ColumnLayout {
    implicitHeight: parent.height
    implicitWidth: parent.width
    spacing: 10

    PersonalizationV1 {
        id: personalization

        onWallpaperChanged: {
            setting.imageSource = "file://" + path;
        }
    }

    WallpaperSetting {
        id: setting
        Layout.alignment: Qt.AlignTop | Qt.AlignVCenter
    }

    Flickable {
        Layout.fillHeight: true
        Layout.fillWidth: true

        clip: true

        contentWidth: width
        contentHeight: localCard.height + systemCard.height

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            WallpaperCard {
                id: localCard
                group: "Local"
                directory: personalization.cacheDirectory

                Layout.alignment: Qt.AlignVCenter
                modelData: personalization.wallpaperModel(group, directory)
                visible: modelData.count > 0
                titleText: qsTr("Local Picture")
                descriptionText: qsTr("Show All") + "-" + modelData.count
                descriptionVisiable: modelData.count > 10
            }

            WallpaperCard {
                id: systemCard
                group: "System"
                directory: "/usr/share/wallpapers/lingmo/"

                Layout.alignment: Qt.AlignVCenter
                modelData: personalization.wallpaperModel(group, directory)
                visible: true
                titleText: qsTr("System Picture")
                descriptionText: qsTr("Show All") + "-" + modelData.count
                descriptionVisiable: modelData.count > 10
            }
        }
    }
}
