// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

DialogWindow {
    id: control
    width: DS.Style.aboutDialog.width
    height: DS.Style.aboutDialog.height

    property alias windowTitle: control.title
    property alias productName: productNameLabel.text
    property alias productIcon: logoLabel.icon.name
    property alias version: versionLabel.text
    property alias description: descriptionLabel.text
    property alias license: licenseLabel.text
    property alias companyLogo: companyLogoLabel.source
    property string websiteName
    property string websiteLink

    readonly property string __websiteLinkTemplate:
        "<a href='%1' style='text-decoration: none; font-size:13px; color: #004EE5;'>%2</a>"

    RowLayout {
        id: contentView
        width: parent.width
        implicitHeight: contentLayout.implicitHeight
        ColumnLayout {
            id: contentLayout
            spacing: 0
            Layout.preferredWidth: Math.max(control.width / 5 * 2, DS.Style.aboutDialog.leftAreaWidth)
            Layout.bottomMargin: DS.Style.aboutDialog.bottomPadding

            D.IconLabel {
                id: logoLabel
                Layout.preferredHeight: DS.Style.aboutDialog.productIconHeight
                Layout.preferredWidth: Layout.preferredHeight
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 0
                display: D.IconLabel.IconOnly
                icon.mode: control.D.ColorSelector.controlState
                icon.theme: control.D.ColorSelector.controlTheme
                icon.palette: D.DTK.makeIconPalette(control.palette)
            }
            Label {
                id: productNameLabel
                font: D.DTK.fontManager.t5
                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 8
            }
            Image {
                id: companyLogoLabel
                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 30
            }
            Label {
                id: licenseLabel
                font: D.DTK.fontManager.t10
                Layout.preferredWidth: parent.width
                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 3
                Layout.leftMargin: 30
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                visible: license !== ""
            }
        }
        ColumnLayout {
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            spacing: 10
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: 20
            ColumnLayout {
                spacing: 1
                Label {
                    font: D.DTK.fontManager.t10
                    text: qsTr("Version")
                }
                Label {
                    id: versionLabel
                    font: D.DTK.fontManager.t8
                    wrapMode: Text.WordWrap
                    text: Qt.application.version
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                spacing: 1
                Label {
                    font: D.DTK.fontManager.t10
                    text: qsTr("Homepage")
                }
                Label {
                    id: websiteLabel
                    font: D.DTK.fontManager.t8
                    text: (control.websiteLink === "" || control.websiteName === "") ?
                              "" : control.__websiteLinkTemplate.arg(websiteLink).arg(control.websiteName)
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                spacing: 1
                Label {
                    font: D.DTK.fontManager.t10
                    text: qsTr("Description")
                }
                Label {
                    id: descriptionLabel
                    Layout.fillWidth: true
                    font: D.DTK.fontManager.t8
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                }
            }
       }

        Component.onCompleted: {
            websiteLabel.linkActivated.connect(D.ApplicationHelper.openUrl)
            descriptionLabel.linkActivated.connect(D.ApplicationHelper.openUrl)
            licenseLabel.linkActivated.connect(D.ApplicationHelper.openUrl)
        }

        focus: true
        Keys.onEscapePressed: {
            control.close()
            event.accepted = true
        }
    }
}
