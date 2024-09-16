// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import org.deepin.dtk 1.0


Item {
    anchors {
        top: parent.top
        left: parent.left
        leftMargin: -20
    }

    width: parent.width + 20
    height: parent.height

    anchors.fill: parent

    property bool bShowImportBtn: false
    property string iconName: "nopicture4"

    ColumnLayout {
        anchors {
            centerIn: parent
        }

        spacing: 15

        ActionButton {
            id: openViewImageIcon
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 140
            Layout.preferredHeight: 100
            hoverEnabled: false
            icon {
            name: iconName
                width:140
                height:100
            }

            MouseArea {
                anchors.fill: parent

                onPressed: (mouse)=> {
                    mouse.accepted = true
                }
            }
        }

        RecommandButton {
            id: openPictureBtn

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 302
            Layout.preferredHeight: 36
            font.capitalization: Font.MixedCase
            text: qsTr("Import Photos and Videos")
            visible: bShowImportBtn
            onClicked:{
                importDialog.open()
            }
        }
        Label {
            Layout.alignment: Qt.AlignHCenter
            color: "#7A7A7A"
            text: bShowImportBtn ? qsTr("Or drag them here") : qsTr("No photos or videos found")
        }
    }
}
