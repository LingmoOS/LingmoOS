// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Rectangle {
    property string iconStr: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEkAAABFCAYAAAAGscunAAAAAXNSR0IArs4c6QAAATNJREFUeF7t3DEOgkAYBeG3J2NvxnIy2JNhKEwUDYyWMJZmLPh8tH+Jn1OBcloYRCQwApH+Rlozgt9eMymZ9g/2uaQ145Ch1dRrIhw81ZIlPX1KSXvNviKNGVt7724BNmVKSxPp6N8WCbwLIokEBEDikkQCAiBxSSIBAZC4JJGAAEhckkhAACQuSSQgABKXJBIQAIlLEgkIgMQliQQEQOKSRAICIHFJIgEBkLgkkYAASFySSEAAJC5JJCAAEpckEhAAiUsSCQiAxCWJBARA4pJEAgIgcUkiAQGQuCSRgABIXJJIQAAkLkkkIAASlyQSEACJSxIJCIDEJYkEBEDikkQCAiBxSSIBAZD8tKSaessLEz09c2ZwPGETX294XuK5tN0Jju1rr96A11AkkYAASFwSQHoAZNuhVXVGph0AAAAASUVORK5CYII="

    RowLayout {
        id:row_layout_1
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        QtIcon {
            name: iconStr
            sourceSize: Qt.size(50, 50)
        }

        DciIcon {
            name: iconStr
            sourceSize: Qt.size(50, 50)
        }

        IconLabel {
            icon.name: iconStr
            icon.width: 50
            icon.height: 50
        }
        QtIcon {
            name: "search_indicator"
            color: "red"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (Qt.colorEqual(parent.color, "red")) {
                        parent.color = "blue"
                    } else {
                        parent.color = "red"
                    }
                }
            }
        }

        BusyIndicator {
            running: true
        }
        BusyIndicator {
            running: true
        }

        RoundButton {
            id:roundbutton
            width: 28
            height: 28
        }

        Dial {}

        Rectangle {
               id: frame
               clip: true
               width: 160
               height: 160
               border.color: "black"

               Text {
                   id: content
                   text: "HELLO WORLD"
                   font.pixelSize: 160
                   x: -hbar.position * width
                   y: -vbar.position * height
               }

               ScrollBar {
                   id: vbar
                   hoverEnabled: true
                   active: hovered || pressed
                   orientation: Qt.Vertical
                   size: frame.height / content.height
                   anchors.top: parent.top
                   anchors.right: parent.right
                   anchors.bottom: parent.bottom

                   ToolTip.visible: hovered
                   ToolTip.delay: 1000
                   ToolTip.timeout: 5000
                   ToolTip.text: "This is a ScrollBar!!!"
               }

               ScrollBar {
                   id: hbar
                   hoverEnabled: true
                   active: hovered || pressed
                   orientation: Qt.Horizontal
                   size: frame.width / content.width
                   anchors.left: parent.left
                   anchors.right: parent.right
                   anchors.bottom: parent.bottom
               }
           }


        StackView {
            id: stackview_1
            width: 150
            height: 150

            Component {
                 id: page
                 Rectangle {
                     property real hue: Math.random()
                     color: Qt.hsla(hue, 0.5, 0.8, 0.6)
                     border.color: Qt.hsla(hue, 0.5, 0.5, 0.9)
                     StackView.visible: true
                 }
            }

            initialItem: page

            MouseArea {
                id:area
                anchors.fill: parent
                onClicked: {
                    stackview_1.push(page)
                }
            }
        }

    }


    RectangularShadow {
        anchors.fill: shadowSource
        glowRadius: 20
        spread: 0
        color: "black"
        cornerRadius: shadowSource.radius
    }

    Rectangle {
        id: shadowSource

        width: 300
        height: 150
        anchors {
            left: parent.left
            leftMargin: 50
            bottom: parent.bottom
            bottomMargin: 200
        }
        radius: 30
        color: "red"
    }

    // test like DropShadow
    BoxShadow {
        anchors.fill: boxShadowSource

        shadowBlur : 20
        shadowColor : Qt.rgba(0, 0, 0, 0.9)
        shadowOffsetX : 0
        shadowOffsetY : 0
        cornerRadius: boxShadowSource.radius
        hollow: true
    }

    Rectangle {
        id: boxShadowSource

        width: 200
        height: 200
        anchors {
            left: shadowSource.right
            leftMargin: 50
            bottom: shadowSource.bottom
        }
        color: "red"
        radius: 20
    }

    // test like InnerShadow
    BoxInsetShadow {
        anchors.fill: boxShadowSource

        shadowBlur : 20
        shadowColor : Qt.rgba(0, 0, 0, 0.6)
        shadowOffsetX : 0
        shadowOffsetY : 0
        cornerRadius: boxShadowSource.radius
    }

    ListView {
        spacing: 10
        anchors {
            left: boxShadowSource.right
            leftMargin: 50
            verticalCenter: boxShadowSource.verticalCenter
        }
        width: 200
        height: 280

        model: ListModel {
            ListElement {
                cornor: 1
            }
            ListElement {
                cornor: 2
            }
            ListElement {
                cornor: 4
            }
            ListElement {
                cornor: 8
            }
            ListElement {
                cornor: 3
            }
            ListElement {
                cornor: 5
            }
            ListElement {
                cornor: 10
            }
            ListElement {
                cornor: 12
            }
        }
        delegate: RoundRectangle {
            width: 200
            height: 60
            color: "blue"
            radius: 20
            antialiasing: true
            corners: cornor
        }
    }

    Rectangle {
        id:swipe_view

        width: 300
        height: 150
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        SwipeView {
            id: view

            currentIndex: 1
            anchors.fill: parent

            Rectangle {
                color: "red"
                width: swipe_view.width
                height: swipe_view.height
            }

            Rectangle {
                color: "green"
                width: swipe_view.width
                height: swipe_view.height
            }

            Rectangle {
                color: "blue"
                width: swipe_view.width
                height: swipe_view.height
            }
        }

        PageIndicator {
            id: indicator

            count: view.count
            currentIndex: view.currentIndex

            anchors.bottom: view.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
