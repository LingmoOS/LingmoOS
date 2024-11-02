
import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Templates 2.5 as T
import org.lingmo.qqc2style.private 1.0 as StylePrivate


T.ScrollView {
    id: controlRoot

    palette: StylePrivate.StyleHelper.palette
    font: StylePrivate.StyleHelper.font
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                             contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                              contentHeight + topPadding + bottomPadding)



    ScrollBar.vertical: ScrollBar {
        parent: controlRoot
        x: controlRoot.mirrored ? 0 : controlRoot.width - width
        y: controlRoot.topPadding
        height: controlRoot.availableHeight
        active:  controlRoot.ScrollBar.horizontal || controlRoot.ScrollBar.horizontal.active

    }

    ScrollBar.horizontal: ScrollBar {
        parent: controlRoot
        x: controlRoot.leftPadding
        y: controlRoot.height - height
        width: controlRoot.availableWidth
        active: controlRoot.ScrollBar.vertical || controlRoot.ScrollBar.vertical.active

    }

}
