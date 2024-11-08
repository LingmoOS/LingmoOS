import QtQuick 2.15
import org.lingmo.menu.core 1.0
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

Loader {
    id: tag
    property bool isFavorited
    signal clicked()

    Component {
        id: editImage
        LingmoUIItems.Button {
            width: 28
            height: 28
            icon.width: 16
            icon.height: 16
            background.paletteRole: Platform.Theme.Light
            background.alpha: 1
            activeFocusOnTab: false

            onClicked: tag.clicked()

            background.radius: width / 2
            icon.source: isFavorited ? "lingmo-cancel-star-symbolic" : "non-starred-symbolic"
        }
    }

    sourceComponent: mainWindow.editMode && (type === DataType.Normal) ? editImage : null
}
