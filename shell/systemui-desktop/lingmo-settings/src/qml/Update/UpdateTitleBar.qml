import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LUI
import Lingmo.Settings 1.0
import "../"

RowLayout {
    id: updatetitlebar
    spacing: LUI.Units.largeSpacing

    signal sendCheckUpdate
    signal requestDownloadandInstall(string name, string package_name, string url, string filename, int index)

    Item {
        width: 80
        height: 80

        Image {
            visible: (!control.isupdating) && (!control.ischeckingupdate)
            source: LUI.Theme.darkMode ? "qrc:/images/dark/changes-white" : "qrc:/images/light/changes"
            width: parent.width
            height: parent.height
            fillMode: Image.PreserveAspectCrop
            antialiasing: true
            smooth: true
            anchors.centerIn: parent
        }

        LUI.BusyIndicator {
            id: busyIndicator
            width: parent.width
            height: parent.height
            visible: (control.isupdating) || (control.ischeckingupdate)
            running: (control.isupdating) || (control.ischeckingupdate)
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        // Center promote text
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Label {
                id: updateText
                text: {
                    if ((!control.isupdating) && (!control.hasupdate_) && (!control.has_error_)) {
                        return qsTr("You're up to date");
                    } else if ((!control.isupdating) && (control.hasupdate_) && (!control.has_error_)) {
                        return qsTr("Updates available");
                    } else if ((control.isupdating)) {
                        return qsTr("Installing updates");
                    } else if ((control.ischeckingupdate)) {
                        return qsTr("Checking updates");
                    } else {
                        return qsTr("Error occurs in updating");
                    }
                }
                Layout.fillHeight: true
                width: parent.width
                font.pointSize: 20
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }

            Label {
                id: updateCheckTimeText
                text: "The last inspection was already the latest"
                Layout.fillHeight: true
                width: parent.width
                font.pointSize: 10
                font.bold: false
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }
        }
    }


    Button {
        id: checkUpdateButton
        width: 40
        text: qsTr("Check for updates")
        visible: !control.hasupdate_
        enabled: !control.ischeckingupdate
        onClicked: {
            control.ischeckingupdate = true;
            // 每次都需要connect 和 disconnect。否则会重复触发？？
            control.mUpdateManager.onUpdateDataReply.connect(control.handle_update_data);
            updatetitlebar.sendCheckUpdate();
        }
    }

    Button {
        id: installUpdateButton
        width: 40
        text: qsTr("Update Now")
        visible: control.hasupdate_
        enabled: !control.isupdating
        onClicked: {
            control.isupdating = true;
            // 执行安装更新
            for (let index=0; index < control.updateListModel.count; index++) {
                let item = control.updateListModel.get(index);
                updatetitlebar.requestDownloadandInstall(item.name, item.package_name, item.download_link, item.filename, index);
            }
        }
    }
}


