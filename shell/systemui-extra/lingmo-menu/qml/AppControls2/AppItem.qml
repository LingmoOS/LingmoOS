import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import "../extensions" as Extension
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

MouseArea {
    id: control
    property bool isSelect: false
    property bool isRecentInstalled: false
    hoverEnabled: true
    states: State {
        when: control.activeFocus
        PropertyChanges {
            target: controlBase
            borderColor: Platform.Theme.Highlight
            border.width: 2
        }
    }

    LingmoUIItems.Tooltip {
        anchors.fill: parent
        mainText: name
        posFollowCursor: true
        margin: 6
        visible: content.textTruncated
    }

    LingmoUIItems.StyleBackground {
        id: controlBase
        anchors.fill: parent
        radius: Platform.Theme.minRadius
        useStyleTransparency: false
        paletteRole: Platform.Theme.Text
        alpha: isSelect ? 1.00 : control.containsPress ? 0.16 : control.containsMouse ? 0.08 : 0.00

        RowLayout {
            anchors.fill: parent
            spacing: 2

            IconLabel {
                id: content
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true

                iconHeight: 32; iconWidth: iconHeight
                appName: name; appIcon: icon
                display: Display.TextBesideIcon
                spacing: 12
            }

            Item {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumWidth: tagLabel.visible ? 28 : isRecentInstalled ? 8 : 0
                Layout.maximumHeight: width
                Layout.rightMargin: tagLabel.visible ? 4 : 0

                Extension.EditModeFlag {
                    id: tagLabel
                    anchors.fill: parent
                    isFavorited: favorite
                    onClicked: {
                        appManager.changeFavoriteState(id, !isFavorited);
                    }
                }

                LingmoUIItems.StyleBackground {
                    id: tagPoint
                    visible: tagLabel.sourceComponent != null ? false : isRecentInstalled
                    anchors.centerIn: parent
                    width: 8; height: width
                    radius: width / 2
                    useStyleTransparency: false
                    paletteRole: Platform.Theme.Highlight
                }
            }
        }
    }
}
