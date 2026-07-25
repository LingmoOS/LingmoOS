import QtQuick
import QtQuick.Controls
import LingmoUI.GraphicalEffects
import LingmoUI


/**
  * This QML module defines a custom rectangle (LingmoRectangle)
  * that uses a layer effect to apply an opacity mask.
  * The opacity mask is defined by a ShaderEffectSource
  * that references another LingmoRectangle with the same dimensions
  * and radius as the parent control. The layer.enabled property is
  * set based on whether the system is running in software mode,
  * and the layer.textureSize is calculated to account for the
  * device's pixel ratio to ensure high-resolution effects.
  */
LingmoRectangle {
    id: control
    color: "#00000000"
    layer.enabled: !LingmoTools.isSoftware()
    layer.textureSize: Qt.size(control.width * 2 * Math.ceil(
                                   Screen.devicePixelRatio),
                               control.height * 2 * Math.ceil(
                                   Screen.devicePixelRatio))
    layer.effect: OpacityMask {
        maskSource: ShaderEffectSource {
            sourceItem: LingmoRectangle {
                radius: control.radius
                width: control.width
                height: control.height
            }
        }
    }
}
