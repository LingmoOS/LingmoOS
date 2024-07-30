// NOTE: check this
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Templates as T

import org.kde.lingmoui as LingmoUI

T.ScrollView {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    data: [
        LingmoUI.WheelHandler {
            target: control.contentItem
        }
    ]

    rightPadding: {
        if (ScrollBar.vertical?.background?.visible) {
            return ScrollBar.vertical.background.width
        } else {
            return horizontalPadding
        }
    }
    bottomPadding: {
        if (ScrollBar.horizontal?.background?.visible) {
            return ScrollBar.horizontal.background.height
        } else {
            return verticalPadding
        }
    }

    ScrollBar.vertical: ScrollBar {
        parent: control
        x: control.mirrored ? 0 : control.width - width
        y: control.topPadding
        height: control.availableHeight
        active: control.ScrollBar.horizontal.active
    }

    ScrollBar.horizontal: ScrollBar {
        parent: control
        x: control.leftPadding
        y: control.height - height
        width: control.availableWidth
        active: control.ScrollBar.vertical.active
    }
}
