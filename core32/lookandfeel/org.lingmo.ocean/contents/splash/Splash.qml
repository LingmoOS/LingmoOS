import QtQuick.Window 2.2
import QtQuick 2.5
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import Lingmo.System 1.0 as System

Rectangle {
    id: root
    color: "#313131"
    property int stage
    onStageChanged: {
        if (stage == 1) {
            introAnimation.running = true
        }
    }

    System.Wallpaper {
        id: wallpaper
    }

    Image {
        id: bg
        source: "/etc/system/wallpaper/img.jpg"
        fillMode: Image.PreserveAspectCrop
        anchors.fill: parent
    }

    FastBlur {
        id: wallpaperBlur
        anchors.fill: bg
        radius: 64
        source: bg
        cached: true
        visible: true
    }

    Item {
        id: content
        anchors.fill: parent
        opacity: 0
        TextMetrics {
            id: units
            text: "M"
            property int gridUnit: boundingRect.height
            property int largeSpacing: units.gridUnit
            property int smallSpacing: Math.max(2, gridUnit/4)
        }

        Rectangle {
            id: imageSource
            width:  300
            height: 300
            color:  "transparent"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            clip: true

            Image {
                id: logo
                //match SDDM/lockscreen avatar positioning
                property real size: units.gridUnit * 8
                anchors.centerIn: parent
                source: "images/logo.svg"
                sourceSize.width: 300
            }

            Image {
                id: busyIndicator
                //in the middle of the remaining space
                y: parent.height - (parent.height - logo.y) / 2
                anchors.horizontalCenter: parent.horizontalCenter
                source: "images/busy.svg"
                sourceSize.height: units.gridUnit * 2
                sourceSize.width: units.gridUnit * 2
                RotationAnimator on rotation {
                    id: rotationAnimator
                    from: 0
                    to: 360
                    duration: 800
                    loops: Animation.Infinite
                }
            }
        }
    }

    OpacityAnimator {
        id: introAnimation
        running: false
        target: content
        from: 0
        to: 1
        duration: 1000
        easing.type: Easing.InOutQuad
    }
}
