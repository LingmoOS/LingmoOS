/*
 *
 *  Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.lingmo.quick.items 1.0
import org.lingmo.windowThumbnail 1.0
import org.lingmo.quick.platform 1.0
import org.lingmo.panel.taskManager 1.0 as LingmoUITaskManager

StyleBackground {
    id: container
    property int spacing: 8
    property var containerIcon
    property bool isList: false
    property bool containsMouse: false
    property bool containsPress: false
    property bool viewIsHorizontal: false
    property string containerTitle: ""

    signal closeButtonClicked()

    paletteRole: Theme.Base
    alpha: !isList ? 0.75 : 0
    useStyleTransparency: false
    radius: Theme.windowRadius
    borderColor: Theme.Text
    borderAlpha: 0.15
    border.width: !isList ? 1 : 0

    Loader {
        id: layoutLoader
        StyleBackground {
            anchors.fill: parent
            paletteRole: Theme.BrightText
            alpha: containsPress ? 0.10 : containsMouse ? 0.05 : 0
            useStyleTransparency: false
            radius: isList ? 4 : Theme.windowRadius
        }
        sourceComponent: isList ? listLayout : thumbnailLayout
    }

    Component {
        id: thumbnailLayout
        ColumnLayout {
            id: baseLayout
            spacing: 0
            height: {
                var preHeight = windowThumbnail.height + container.spacing * 3 + 24;
                if (preHeight < minimumHeight ) {
                    return minimumHeight;
                } else if (preHeight > maximumHeight) {
                    windowThumbnail.Layout.maximumHeight = maximumHeight - container.spacing * 3 - 24;
                    return maximumHeight
                } else {
                    return preHeight;
                }
            }
            width: {
                var preWidth = windowThumbnail.width + container.spacing * 2;
                if (preWidth < minimumWidth ) {
                    return minimumWidth;
                } else if (preWidth > maximumWidth) {
                    windowThumbnail.Layout.maximumWidth = maximumWidth - container.spacing * 2;
                    return maximumWidth
                } else {
                    return preWidth;
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
                Layout.maximumHeight: 24
                Layout.margins: container.spacing
                Layout.bottomMargin: 0

                Icon {
                    Layout.preferredHeight: 24
                    Layout.preferredWidth: 24
                    Layout.rightMargin: container.spacing
                    source: containerIcon
                }
                StyleText {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    text: containerTitle
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
                MouseArea {
                    Layout.preferredHeight: 16
                    Layout.preferredWidth: 16
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    hoverEnabled: true
                    visible: container.containsMouse

                    Rectangle {
                        color: "red"
                        radius: 4
                        anchors.fill: parent
                        visible: parent.containsMouse
                    }
                    Icon {
                        anchors.fill: parent
                        mode: parent.containsMouse ? Icon.Highlight : Icon.AutoHighlight
                        source: "window-close-symbolic"
                    }
                    onClicked: {
                        closeButtonClicked();
                    }
                }
            }

            WindowThumbnail {
                id: windowThumbnail
                Layout.alignment: Qt.AlignHCenter
                Layout.margins: container.spacing
                Layout.preferredWidth: viewIsHorizontal ? itemWidth : viewLoader.preChildrenSize - 16
                Layout.preferredHeight: viewIsHorizontal ? viewLoader.preChildrenSize - 48 : itemHeight

                property real itemHeight
                property real itemWidth
                // 窗口visible改变
                winId: windowVisible ? modelData : ""

                function updateSize() {
                    if (winId === "") {
                        return;
                    }
                    var itemGeometry = WindowManager.geometry(winId);
                    if (viewIsHorizontal) {
                        itemWidth = itemGeometry.width * ((viewLoader.preChildrenSize - 48 ) / itemGeometry.height);
                    } else {
                        itemHeight = itemGeometry.height * ((viewLoader.preChildrenSize - 16) / itemGeometry.width);
                    }
                }
                onWinIdChanged: updateSize()
                onWidthChanged: updateSize()
                onHeightChanged: updateSize()
            }
        }
    }
    Component {
        id: listLayout
        Item {
            width: 220
            height: 36
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8

                Icon {
                    Layout.preferredHeight: 16
                    Layout.preferredWidth: 16
                    source: containerIcon
                }
                StyleText {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    text: containerTitle
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
                MouseArea {
                    Layout.preferredHeight: 16
                    Layout.preferredWidth: 16
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    hoverEnabled: true
                    visible: container.containsMouse

                    Rectangle {
                        color: "red"
                        radius: 4
                        anchors.fill: parent
                        visible: parent.containsMouse
                    }
                    Icon {
                        anchors.fill: parent
                        source: "window-close-symbolic"
                    }
                    onClicked: {
                        closeButtonClicked();
                    }
                }
            }
        }
    }
}
