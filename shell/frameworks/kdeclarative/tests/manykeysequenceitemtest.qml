import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmoui 2.11 as LingmoUI
import org.kde.kquickcontrols 2.0

LingmoUI.ApplicationWindow {
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
