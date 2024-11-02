import QtQuick 2.15
import QtQuick.Controls 1.4
import org.lingmo.quick.items 1.0
Row {
    property alias name: nameText.text
    property alias value: valueText.text
    width: childrenRect.width
    height: childrenRect.height
    spacing: 10
    StyleText {
        id: nameText
        wrapMode: Text.Wrap
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
    StyleText {
        id: valueText
        wrapMode: Text.Wrap
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}