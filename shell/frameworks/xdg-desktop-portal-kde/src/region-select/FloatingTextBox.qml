import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.Padding {
    // Reusable/shared object
    required property FontMetrics fontMetrics

    padding: Kirigami.Units.mediumSpacing * 2
    verticalPadding: padding - fontMetrics.descent

    FloatingBackground {
        anchors.fill: parent
        z: -1

        radius: Kirigami.Units.mediumSpacing / 2 + border.width

        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.85)
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.2)
        border.width: 1
    }
}
