import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.Padding {
    // Reusable/shared object
    required property FontMetrics fontMetrics

    padding: LingmoUI.Units.mediumSpacing * 2
    verticalPadding: padding - fontMetrics.descent

    FloatingBackground {
        anchors.fill: parent
        z: -1

        radius: LingmoUI.Units.mediumSpacing / 2 + border.width

        color: Qt.alpha(LingmoUI.Theme.backgroundColor, 0.85)
        border.color: Qt.alpha(LingmoUI.Theme.textColor, 0.2)
        border.width: 1
    }
}
