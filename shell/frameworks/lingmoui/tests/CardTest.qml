import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

Item {
    width: 600
    height: 600

    LingmoUI.Card {
        width: 300
        anchors.centerIn: parent

        banner.title: "Card"
        banner.titleIcon: "document-new"
        banner.titleAlignment: alignCombo.currentValue
        banner.source: "/usr/share/wallpapers/Next/contents/screenshot.png"

        contentItem: Label {
            text: "Card Contents"
        }

        actions: [
            LingmoUI.Action {
                icon.name: "document-new"
                text: "Action 1"
            },
            LingmoUI.Action {
                icon.name: "document-new"
                text: "Action 2"
            }
        ]
    }

    RowLayout {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        ComboBox {
            id: alignCombo

            model: [
                { text: "Top Left", align: Qt.AlignLeft | Qt.AlignTop },
                { text: "Top Center", align: Qt.AlignHCenter | Qt.AlignTop },
                { text: "Top Right", align: Qt.AlignRight | Qt.AlignTop },
                { text: "Center Left", align: Qt.AlignLeft | Qt.AlignVCenter },
                { text: "Center", align: Qt.AlignHCenter | Qt.AlignVCenter },
                { text: "Center Right", align: Qt.AlignRight | Qt.AlignVCenter },
                { text: "Bottom Left", align: Qt.AlignLeft | Qt.AlignBottom },
                { text: "Bottom Center", align: Qt.AlignHCenter | Qt.AlignBottom },
                { text: "Bottom Right", align: Qt.AlignRight | Qt.AlignBottom }
            ]

            textRole: "text"
            valueRole: "align"
        }
    }
}
