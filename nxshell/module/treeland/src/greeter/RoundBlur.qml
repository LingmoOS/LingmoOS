import QtQuick
import org.lingmo.dtk 1.0 as D

D.BoxPanel {
    width: parent.width
    height: parent.height
    radius: width / 2
    color1: D.Palette {
        normal: Qt.rgba(1, 1, 1, 0.3)
    }

    color2: D.Palette {
        normal: Qt.rgba(1, 1, 1, 0.3)
    }
    boxShadowBlur: 12
    backgroundFlowsHovered: false
    boxShadowOffsetY: 0
    innerShadowOffsetY1: 0
    insideBorderColor: D.Palette {
        normal: Qt.rgba(1, 1, 1, 0)
    }

    outsideBorderColor: D.Palette {
        normal: Qt.rgba(1, 1, 1, 0)
    }

    dropShadowColor: D.Palette {
        normal: Qt.rgba(1, 1, 1, 0)
    }

    innerShadowColor1: D.Palette {
        normal: Qt.rgba(1, 1, 1, 0)
    }

    innerShadowColor2: D.Palette {
        normal: Qt.rgba(1, 1, 1, 0)
    }
}
