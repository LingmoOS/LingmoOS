import org.kde.lingmoui as LingmoUI

LingmoUI.Page {
    readonly property bool isHome: true

    title: placeholder.text

    LingmoUI.LoadingPlaceholder {
        id: placeholder
        anchors.centerIn: parent
    }
}
