// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0

Item {
    id: control

    anchors.fill: parent

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 10

        DciIcon {
            id: openWidgetImage

            Layout.alignment: Qt.AlignCenter
            name: "import_photo"
            sourceSize.height: 128
            sourceSize.width: 128
            theme: DTK.themeType
        }

        RecommandButton {
            id: openFileBtn

            Layout.preferredHeight: 35
            Layout.preferredWidth: 300
            font.capitalization: Font.MixedCase
            text: qsTr("Open Image")

            onClicked: stackView.openImageDialog()
        }
    }
}
