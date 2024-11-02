import QtQuick 2.0

Item {
    id: scannerContainer
    anchors.fill: parent
    anchors.leftMargin: 0
    Behavior on anchors.leftMargin { NumberAnimation { duration: scannerVariables.imageAniDuration } }
    anchors.bottomMargin: 0
    Behavior on anchors.bottomMargin { NumberAnimation { duration: scannerVariables.imageAniDuration } }
    anchors.topMargin: 0
    Behavior on anchors.topMargin { NumberAnimation { duration: scannerVariables.imageAniDuration } }
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
        propagateComposedEvents:true //事件在组件间进行传递

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
        id: scannerImageloader
        objectName: "scannerImageLoader"
        visible: true
        source: "ShowScannerImage.qml"
    }
}
