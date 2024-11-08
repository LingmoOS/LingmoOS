import QtQuick 2.0
import org.lingmo.menu.core 1.0
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

Item {
    id: root

    property int display: Display.TextUnderIcon
    // 上下布局icon大小为48*48  左右布局icon大小为32*32
    property int iconHeight: 48
    property int iconWidth: 48
    property string appName: ""
    property string appIcon: ""
    // 上下布局spacing：0  左右布局spacing：12
    property int spacing: 2
    property bool textTruncated: iconText.truncated
    property bool textHighLight: false

    state: root.display

    states: [
        State {
            name: Display.TextBesideIcon
            PropertyChanges {
                target: iconImage
                anchors.verticalCenter: root.verticalCenter
                anchors.left: root.left
                anchors.leftMargin: root.spacing
            }
            PropertyChanges {
                target: iconText
                width: root.width - iconImage.width - root.spacing * 2

                horizontalAlignment: Text.AlignLeft
                anchors.left: iconImage.right
                anchors.leftMargin: root.spacing
                anchors.verticalCenter: root.verticalCenter
            }
        },
        State {
            name: Display.TextUnderIcon
            PropertyChanges {
                target: iconImage
                anchors.top: root.top
                anchors.topMargin: (root.height - root.spacing - iconImage.height - iconText.height) / 2
                anchors.horizontalCenter: root.horizontalCenter
            }
            PropertyChanges {
                target: iconText
                width: root.width

                horizontalAlignment: Text.AlignHCenter
                anchors.top: iconImage.bottom
                anchors.topMargin: root.spacing
                anchors.horizontalCenter: root.horizontalCenter
            }
        }
    ]

    LingmoUIItems.Icon {
        id: iconImage
        height: root.iconHeight
        width: root.iconWidth
        source: root.appIcon
    }

    LingmoUIItems.StyleText {
        id: iconText
        text: root.appName
        elide: Text.ElideRight
        paletteRole: root.textHighLight ? Platform.Theme.HighlightedText : Platform.Theme.Text
    }
}
