import QtQuick
import LingmoUI.GraphicalEffects
import LingmoUI

Item {
    id: control
    property color tintColor: Qt.rgba(1, 1, 1, 1)
    property real tintOpacity: 0.65
    property real luminosity: 0.01
    property real noiseOpacity: 0.02
    property var target
    property int blurRadius: 32
    property rect targetRect: Qt.rect(control.x, control.y, control.width, control.height)
    // 四角圆角大小
    property int cornerRadius: LingmoUnits.windowRadius

    Rectangle { // 包围整个内容的矩形，用于创建圆角效果
        id: roundedBackground
        anchors.fill: parent
        radius: control.cornerRadius
        clip: true // 启用剪裁以应用圆角效果
        color: "transparent" // 设置为透明，因为我们只关心它的圆角和剪裁

        ShaderEffectSource {
            id: effect_source
            anchors.fill: parent
            visible: false
            sourceRect: control.targetRect
            sourceItem: control.target
        }

        LingmoClip {
            // 包围整个内容的矩形，用于为Blur创建圆角效果
            anchors.fill: parent
            radius: {
                return new Array(4).fill(control.cornerRadius + 1);
            }

            FastBlur {
                id: fast_blur
                anchors.fill: parent
                source: effect_source
                radius: control.blurRadius
            }
        }

        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(1, 1, 1, luminosity)
            radius: parent.radius // 继承父级的圆角半径
        }
        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(tintColor.r, tintColor.g, tintColor.b, tintOpacity)
            radius: parent.radius // 继承父级的圆角半径
        }
        Image {
            id: noise_image
            anchors.fill: parent
            source: "qrc:/lingmoui/kit/LingmoUI/Image/noise.png"
            fillMode: Image.Tile
            opacity: control.noiseOpacity
        }
    }
}
