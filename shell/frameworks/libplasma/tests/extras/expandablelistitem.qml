import QtQuick
import QtQuick.Layouts

import QtQml.Models
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

Rectangle {
    height: 800
    width: 500
    color: Kirigami.Theme.backgroundColor
    PlasmaComponents.ScrollView {
        anchors.fill: parent
        ListView {
            anchors.fill: parent
            focus: true
            currentIndex: -1
            clip: true
            model: myModel
            highlight: PlasmaExtras.Highlight {}
            highlightMoveDuration: Kirigami.Units.longDuration
            highlightResizeDuration: Kirigami.Units.longDuration
            delegate: PlasmaExtras.ExpandableListItem {
                title: model.title
                subtitle: model.subtitle
                icon: model.icon
                isBusy: model.busy
                subtitleCanWrap: model.subtitleCanWrap || false

                customExpandedViewContent: Component {
                    ColumnLayout {
                        PlasmaComponents.Label {
                            text: "I am some expanded text"
                        }
                        PlasmaComponents.Button {
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
