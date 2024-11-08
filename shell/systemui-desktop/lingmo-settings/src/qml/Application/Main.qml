import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import LingmoUI 1.0 as LingmoUI
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Application")

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: LingmoUI.Units.smallSpacing

            Label {
                text: qsTr("Default application")
                leftPadding: LingmoUI.Units.largeSpacing
            }

            RoundedItem {
                GridLayout {
                    columns: 2
                    columnSpacing: LingmoUI.Units.largeSpacing * 2

                    Label {
                        text: qsTr("Web browser")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("File manager")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Email")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Terminal emulator")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
