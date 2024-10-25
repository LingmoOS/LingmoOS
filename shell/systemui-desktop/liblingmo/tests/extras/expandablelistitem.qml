import QtQuick
import QtQuick.Layouts

import QtQml.Models
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

Rectangle {
    height: 800
    width: 500
    color: LingmoUI.Theme.backgroundColor
    LingmoComponents.ScrollView {
        anchors.fill: parent
        ListView {
            anchors.fill: parent
            focus: true
            currentIndex: -1
            clip: true
            model: myModel
            highlight: LingmoExtras.Highlight {}
            highlightMoveDuration: LingmoUI.Units.longDuration
            highlightResizeDuration: LingmoUI.Units.longDuration
            delegate: LingmoExtras.ExpandableListItem {
                title: model.title
                subtitle: model.subtitle
                icon: model.icon
                isBusy: model.busy
                subtitleCanWrap: model.subtitleCanWrap || false

                customExpandedViewContent: Component {
                    ColumnLayout {
                        LingmoComponents.Label {
                            text: "I am some expanded text"
                        }
                        LingmoComponents.Button {
                            text: "with an expanded button"
                        }
                    }
                }
            }
        }
    }

    ListModel {
        id: myModel
        ListElement {
            title: "Item 1"
            subtitle: "Default with icon"
            icon: "system-file-manager"
            isDefault: true
        }
        ListElement {
            title: "Item 2"
            subtitle: "A really long subtitle that probably won't fit in this constrained example because of how long it is."
            isDefault: false
        }
        ListElement {
            title: "Item 4"
            subtitle: "Busy"
            isDefault: false
            busy: true
        }

    }
}
