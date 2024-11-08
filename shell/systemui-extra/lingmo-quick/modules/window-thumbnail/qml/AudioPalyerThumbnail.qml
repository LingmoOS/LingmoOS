/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: qiqi <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.15
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0
import org.lingmo.windowThumbnail 1.0

Repeater {
    id: view
    property var playerData
    property int windowRadius
    property bool containsMouse: false
    property bool isOpenGL: true
    signal closeButtonClicked()
    signal favoriteButtonClicked()

    model: playerData
    RowLayout {
        id: musicBase
        width: 304
        height: 96

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: height
            Loader {
                id: artUrlImage
                anchors.margins: 4
                anchors.fill: parent
                property var iconSource: model.MetaData["mpris:artUrl"] ? model.MetaData["mpris:artUrl"] : "qrc:/image/default-image.svg"
                sourceComponent: isOpenGL ? radiusIcon : icon
            }
            Component {
                id: radiusIcon
                Icon {
                    source: iconSource
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
                            width: artUrlImage.width
                            height: artUrlImage.height
                            radius: windowRadius - 4
                        }
                    }
                }
            }
            Component {
                id: icon
                Icon {
                    source: iconSource
                }
            }
        }

        // 名称 + 歌词 + 按钮
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 12
                anchors.bottomMargin: 8
                anchors.topMargin: 8
                spacing: 0
                StyleText {
                    Layout.fillWidth: true
                    text: model.MetaData["xesam:title"] ? model.MetaData["xesam:title"] : ""
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
                StyleText {
                    Layout.fillWidth: true
                    text: model.MetaData["xesam:artist"] ? model.MetaData["xesam:artist"] : ""
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
                MprisPlayerButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    playStatus: model.PlaybackStatus
                    onSkipBackwardClicked: {
                        playerData.operation(view.model.index(index, 0), MprisProperties.Previous, [])
                    }
                    onPlayStatusClicked: {
                        playerData.operation(view.model.index(index, 0), MprisProperties.PlayPause, [])
                    }
                    onSkipForwardClicked: {
                        playerData.operation(view.model.index(index, 0), MprisProperties.Next, [])
                    }
                }
            }
        }

        // 关闭按钮 + 收藏按钮
        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: 32 + 4

            MouseArea {
                width: 32
                height: width
                anchors.top: parent.top
                anchors.topMargin: 4
                anchors.right: parent.right
                anchors.rightMargin: 4
                hoverEnabled: true
                visible: view.containsMouse

                Rectangle {
                    color: "red"
                    radius: 8
                    anchors.fill: parent
                    visible: parent.containsMouse
                }
                Icon {
                    width: parent.width / 2
                    height: parent.height / 2
                    anchors.centerIn: parent
                    mode: parent.containsMouse ? Icon.Highlight : Icon.AutoHighlight
                    source: "window-close-symbolic"
                }
                onClicked: {
                    closeButtonClicked();
                }
            }

            //TODO:收藏按钮
            MouseArea {
                width: 16
                height: width
                anchors.left: parent.left
                anchors.leftMargin: 4
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 14
                onClicked: favoriteButtonClicked()

                Icon {
                    anchors.fill: parent
                    //source: ""
                }
            }
        }
    }
}
