import QtQuick 2.0

Item {
    id: mattingContainer
    anchors.fill: parent
    anchors.leftMargin: 0
    Behavior on anchors.leftMargin { NumberAnimation { duration: mattingvariables.imageAniDuration } }
    anchors.bottomMargin: 0
    Behavior on anchors.bottomMargin { NumberAnimation { duration: mattingvariables.imageAniDuration } }
    anchors.topMargin: 0
    Behavior on anchors.topMargin { NumberAnimation { duration: mattingvariables.imageAniDuration } }
    anchors.rightMargin: 0

    signal zoomIn(var wheelDelta)
    signal zoomOut(var wheelDelta)
    signal zoomReset()
    signal zoomActual()
    signal rotate(var deg)
    signal rotateReset()
    signal mirrorH()
    signal mirrorV()
    signal mirrorReset()

    MouseArea {
        id: transMouse
        anchors.fill: parent
        propagateComposedEvents:true
        onPositionChanged: {
            mouse.accepted = false
        }
        onDoubleClicked: {
            mouse.accepted = false
        }
        onClicked: {
            mouse.accepted = false
        }
        onReleased: {
            mouse.accepted = false
        }
    }

    Loader {
        id: mattingImageloader
        objectName: "mattingImageLoader"
        visible: true
        source: "ShowMattingImage.qml"
    }
}
