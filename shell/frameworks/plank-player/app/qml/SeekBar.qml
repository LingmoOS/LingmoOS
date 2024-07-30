// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as QQCT
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.15 as Controls

Item {
    id: seekControl
    property int duration
    property int seekPosition
    property int value: 0

    onActiveFocusChanged: {
        if(activeFocus){
            slider.forceActiveFocus()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        QQCT.Slider {
            id: slider
            Layout.fillWidth: true
            Layout.fillHeight: true
            value: seekControl.value
            to: seekControl.duration || 100
            property int minimumValue: 0
            property int maximumValue: 20
            property bool navSliderItem

            onMoved: {
                video.seek(value);
            }

            onNavSliderItemChanged: {
                if(slider.navSliderItem){
                    recthandler.color = "red"
                } else if (slider.focus) {
                    recthandler.color = Kirigami.Theme.linkColor
                }
            }

            onFocusChanged: {
                if(!slider.focus){
                    recthandler.color = Kirigami.Theme.textColor
                } else {
                    recthandler.color = Kirigami.Theme.linkColor
                }
            }

            handle: Rectangle {
                id: recthandler
                x: slider.position * (parent.width - width)
                implicitWidth: Kirigami.Units.largeSpacing
                implicitHeight: parent.height
                radius: 6
                color: Kirigami.Theme.textColor
            }

            background: Item {
                Rectangle {
                    id: groove
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        right: parent.right
                    }
                    height: parent.height
                    radius: 6
                    color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.3)
                    Rectangle {
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                        }
                        radius: 6
                        color: Kirigami.Theme.highlightColor
                        width: slider.position * (parent.width - slider.handle.width/2) + slider.handle.width/2
                    }
                }
            }

            Keys.onReturnPressed: (event)=> {
                if(!navSliderItem){
                    navSliderItem = true
                } else {
                    navSliderItem = false
                }
            }

            Keys.onLeftPressed: (event)=> {
                if(navSliderItem) {
                    video.seek(video.position - 5000)
                } else {
                    mediaSkipFwdButton.forceActiveFocus()
                }
            }

            Keys.onRightPressed: (event)=> {
                if(navSliderItem) {
                    video.seek(video.position + 5000)
                } else  {
                    pinOsdButton.forceActiveFocus()
                }
            }

        }

        Controls.Label {
            Layout.preferredWidth: contentWidth + Kirigami.Units.gridUnit
            Layout.fillHeight: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: formatTime(duration)
            color: "white"
        }
    }

    function formatTime(timeInMs) {
        if (!timeInMs || timeInMs <= 0) return "0:00"
        var seconds = timeInMs / 1000;
        var minutes = Math.floor(seconds / 60)
        seconds = Math.floor(seconds % 60)
        if (seconds < 10) seconds = "0" + seconds;
        return minutes + "." + seconds
    }
}
