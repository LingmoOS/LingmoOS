
import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Templates 2.5 as T
import org.lingmo.qqc2style.private 1.0 as StylePrivate
import QtQuick.Controls 2.5

T.MenuItem {
    id: controlRoot

    palette: StylePrivate.StyleHelper.palette 
    font: StylePrivate.StyleHelper.font

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding + (arrow ? arrow.implicitWidth : 0))
    implicitHeight: visible ? Math.max(background ? background.implicitHeight : 0,
                             Math.max(contentItem.implicitHeight,
                                      indicator ? indicator.implicitHeight : 0) + topPadding + bottomPadding) : 0
    baselineOffset: contentItem.y + contentItem.baselineOffset

    width: parent ? parent.width : implicitWidth

    Layout.fillWidth: true
    padding: 6


    property int gridUnit: fontMetrics.height
    property string icontype: "default"


    leftPadding: Math.floor(gridUnit/4)*2
    rightPadding: Math.floor(gridUnit/4)*2
    hoverEnabled: true

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))

        onActivated: {
            if (controlRoot.checkable) {
                controlRoot.toggle();
            } else {
                controlRoot.clicked();
            }
        }
    }

    contentItem: RowLayout {
        Item {
           Layout.preferredWidth: (controlRoot.ListView.view && controlRoot.ListView.view.hasCheckables) || controlRoot.checkable ? controlRoot.indicator.width : Math.floor(gridUnit/4)
        }

        StylePrivate.KyIcon {
            iconName: controlRoot.icon.name
            width: controlRoot.icon.width
            height: controlRoot.icon.height
            sunken: controlRoot.pressed || (controlRoot.checkable && controlRoot.checked)
            hover: controlRoot.hovered
            hasFocus: controlRoot.activeFocus
            icontype: controlRoot.icontype
        }
        Label {
            id: label
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            text: controlRoot.text
            font: controlRoot.font

            elide: Text.ElideRight
            visible: controlRoot.text
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
        Label {
            id: shortcut
            Layout.alignment: Qt.AlignVCenter

            visible: controlRoot.action && controlRoot.action.hasOwnProperty("shortcut") && controlRoot.action.shortcut !== undefined
            text: visible ? controlRoot.action.shortcut : ""
            font: controlRoot.font
            color: label.color
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }
        Item {
           Layout.preferredWidth: Math.floor(gridUnit/4)
        }
    }

    background: Item {
        anchors.fill: parent
        implicitWidth: gridUnit * 8

        Rectangle {
            anchors.fill: parent
            opacity: (controlRoot.highlighted || controlRoot.hovered) ? 1 : 0

            color: controlRoot.palette.highlight
        }
    }
    property variant fontMetrics: TextMetrics {
        text: "M"
        function roundedIconSize(size) {
            if (size < 16) {
                return size;
            } else if (size < 22) {
                return 16;
            } else if (size < 32) {
                return 22;
            } else if (size < 48) {
                return 32;
            } else if (size < 64) {
                return 48;
            } else {
                return size;
            }
        }
    }
}





