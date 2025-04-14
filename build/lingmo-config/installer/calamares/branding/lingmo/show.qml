import QtQuick 2.0;
import calamares.slideshow 1.0;

Presentation
{
    id: presentation

    Timer {
        id: slideTimer
        interval: 20000
        repeat: true
        running: true
        onTriggered: presentation.goToNextSlide()
    }

    Slide {
        Image {
            id: background1
            source: "welcome.png"
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
        }

        Text {
            anchors.horizontalCenter: background1.horizontalCenter
            anchors.top: background1.bottom
            text: qsTr("Welcome to Lingmo OS.<br/>" +
                       "This is the beta version")
            wrapMode: Text.WordWrap
            width: parent.width * 0.8
            horizontalAlignment: Text.Center
            visible: false
        }
    }
}

