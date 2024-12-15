import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
    id: root

    property QtObject effectFrame: null

    width: effectFrame ? effectFrame.size.width : 0
    height: effectFrame ? effectFrame.size.height : 0
    radius: effectFrame ? effectFrame.radius : 0
    color: "transparent"

    Image {
        id: icon
        width: root.width
        height: root.height
        source: effectFrame ? effectFrame.image : ""

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Item {
                width: icon.width
                height: icon.height
                Rectangle {
                    anchors.centerIn: parent
                    width: icon.width
                    height: icon.height
                    radius: root.radius
                }
            }
        }
    }
}