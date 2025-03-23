// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0

Item {
    Image {
        id: back
        anchors.fill: parent
        source: "file:///usr/share/wallpapers/deepin/desktop.jpg"
    }

    Rectangle {
        id: source
        y: 50
        x: 50
        width: 200
        height: 200
        radius: 30
        color: "red"
        visible: false
    }

    Item {
        id: maskSource
        anchors.fill: source
        visible: false
        Rectangle {
            anchors.centerIn: parent
            width: 100
            height: 100
            radius: 40
            color: "blue"
        }
    }

    OpacityMask {
        anchors.fill: source
        source: source
        maskSource: maskSource
        invert: true
    }

    BoxShadow {
        hollow: true
        anchors.fill: shadowedRect
        shadowBlur: 20
        shadowColor: "red"
        topLeftRadius: shadowedRect.radius
        bottomRightRadius: shadowedRect.radius
    }
    RoundRectangle {
        id: shadowedRect
        x: 50
        y: 300
        width: 100
        height: 100
        color: "blue"
        radius: 30
        corners: RoundRectangle.TopLeftCorner | RoundRectangle.BottomRightCorner
    }

    ListView {
        id: testView

        property color foreground: "#55ffffff"
        model: 1
        spacing: 30
        clip: true
        anchors {
            fill: parent
            margins: 10
        }

        delegate: Item {
            property ListView view: ListView.view

            height: 50
            width: 300
            anchors.horizontalCenter: parent.horizontalCenter

            InWindowBlur {
                id: blur
                anchors.fill: parent
                radius: 20
                offscreen: true
            }

            ItemViewport {
                id: roundBlur
                anchors.fill: blur
                fixed: true
                sourceItem: blur
                radius: 20
                hideSource: false
            }

            Rectangle {
                radius: roundBlur.radius
                anchors.fill: roundBlur
                color: view.foreground
            }

            Text {
                anchors.centerIn: parent
                text: "InWindowBlur" + index
                color: "white"
                font.pixelSize: 22
            }
        }
    }

    Button {
        text: "深浅切换"
        onClicked: {
            if (testView.foreground == "#55000000")
                testView.foreground = "#55ffffff"
            else
                testView.foreground = "#55000000"
        }
    }

//    StyledArrowShapeBlur {
//        anchors.centerIn: parent
//        width: 100
//        height: 200
//        roundJoinRadius: 20
//    }
}
