import QtQuick 2.15
import org.lingmo.quick.platform 1.0
import org.lingmo.quick.items 1.0
Row {
    width: 900
    height: 900
    spacing: 4
    StyleBackground {
        width: 80
        height: 80
        radius: 12
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: 0.5
        Tooltip {
            width: parent.width
            height: parent.height
            mainText: "tooltip test 1"
            posFollowCursor: false
            location: Dialog.TopEdge
            margin: 9
        }
    }
    StyleBackground {
        width: 80
        height: 80
        radius: 12
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: 0.5
        Tooltip {
            width: parent.width
            height: parent.height
            mainText: "tooltip test 2 \n"
            posFollowCursor: false
            location: Dialog.TopEdge
            margin: 9
        }
    }
    StyleBackground {
        width: 80
        height: 80
        radius: 12
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: 0.5
        Tooltip {
            width: parent.width
            height: parent.height
            mainText: "tooltip test 3 tooltip test 3 tooltip test 3 tooltip test 3"
            posFollowCursor: false
            location: Dialog.TopEdge
            margin: 9
        }
    }
}