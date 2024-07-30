import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.11 as Kirigami
import org.kde.kquickcontrols 2.0

Kirigami.ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Test")

    ColumnLayout {
        KeySequenceItem {
            checkForConflictsAgainst: ShortcutType.None
            modifierlessAllowed: true
        }
        KeySequenceItem {
            checkForConflictsAgainst: ShortcutType.None
            modifierlessAllowed: true
        }
        KeySequenceItem {
            checkForConflictsAgainst: ShortcutType.None
            modifierlessAllowed: true
        }
    }
}
