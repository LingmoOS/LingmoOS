import QtQuick 2.0

import org.kde.kquickcontrols 2.0
import QtQuick.Layouts 1.1

RowLayout {

    KeySequenceItem
    {
        id: sequenceItem
        modifierOnlyAllowed: true
    }
    Text
    {
        text: sequenceItem.keySequence
    }
}
