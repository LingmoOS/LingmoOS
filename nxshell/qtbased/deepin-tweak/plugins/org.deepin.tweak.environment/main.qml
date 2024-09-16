// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.7
import Qt.labs.qmlmodels 1.0
import Qt.labs.platform 1.1
import org.deepin.dtk 1.0
import org.deepin.tweak 1.0

ColumnLayout {
    id: frame
    spacing: 10
    property string displayName: qsTr("System Environment")
    property string description: qsTr("change system environment for user")
    property string version: "1.0.0"
    property string author: "justforlxz"
    property string icon: "icon.png"

    TableView {
        id: tableView
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 10
        columnSpacing: 1
        rowSpacing: 1
        clip: true

        model: TableModel {
            TableModelColumn { display: "key" }
            TableModelColumn { display: "value" }
            rows: []
        }

        delegate: Rectangle {
            implicitHeight: 30
            implicitWidth: tableView.width /  2 - 1
            border.width: 1

            Text {
                text: display
                anchors.centerIn: parent
                visible: !loader.visible
            }
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    if(mouse.button == Qt.RightButton) {
                        if(timer.running)
                        {
                            mouseArea.onDoubleClicked()
                            timer.stop()
                        }
                        else {
                            timer.restart()
                        }
                    }
                }
                onDoubleClicked: {
                    loader.visible = true
                    loader.item.forceActiveFocus()
                }
                Timer{
                    id:timer
                    interval: 200
                    onTriggered: mouseArea.onClicked()
                }
            }
            Loader {
                id: loader
                anchors { verticalCenter: parent.verticalCenter; left: parent.left}
                height: parent.height
                width: parent.width
                visible: false
                sourceComponent: visible ? input : undefined

                Component {
                    id: input
                    TextField {
                        anchors { fill: parent }
                        text: display
                        onAccepted:{
                            model.display = item.text
                            loader.visible = false
                            add.clicked()
                        }
                        validator: RegExpValidator {}
                        onActiveFocusChanged: {
                            if (!activeFocus) {
                                loader.visible = false
                            }
                            if (model.column === 0) {
                                item.validator.regExp = /^[a-zA-Z][a-zA-Z0-9_]*$/
                            }
                            else {
                                item.validator.regExp = '*'
                            }
                        }
                        Keys.onEscapePressed: loader.visible = false
                    }

                }
            }
        }
    }

    Text {
        id: tips
        Layout.fillWidth: true
        Layout.margins: 10
        wrapMode: Text.WordWrap
        text: qsTr('Please modify environment variables carefully.You must clearly understand the importance of system environment variables. This operation is irreversible.If there is a problem, please log in with tty and delete the $HOME/.dde_env file to restore the environment. Please login again to apply changes.')
    }

    Button {
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        id: add
        text: qsTr("Add")
        onClicked: {
            // TODO: remove empty object inside.
            for (const row of tableView.model.rows) {
                if (row['key'] === '' && row['value'] === '') {
                    return;
                }
            }

            tableView.model.appendRow({key: '', value: ''})
        }
    }

    Button {
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        Layout.bottomMargin: 10
        text: qsTr("Save")
        onClicked: {
            const paths = StandardPaths.standardLocations(StandardPaths.HomeLocation)
            if (paths.length === 0) {
                return;
            }
            const path = paths[0] + '/.dde_env';
            const file = Tweak.newFile(path.slice(6, path.length));
            if (!file.open(FileMode.WriteOnly)) {
                console.error('cannot open .dde_env on $HOME');
                return;
            }

            const datas = tableView.model.rows;
            console.log(JSON.stringify(datas));
            for (const data of datas) {
                if (data['key'] === '' || data['value'] === '') {
                    continue;
                }
                file.write('export ' + data['key'] + '=' + data['value'] + '\n');
            }
            file.close();
        }
    }

    Component.onCompleted: {
        const paths = StandardPaths.standardLocations(StandardPaths.HomeLocation)
        if (paths.length === 0) {
            return;
        }
        const path = paths[0] + '/.dde_env';
        const file = Tweak.newFile(path.slice(6, path.length));
        if (!file.open(FileMode.ReadOnly)) {
            console.error('cannot open .dde_env on $HOME');
            add.clicked();
            return;
        }
        const s = new String(file.readAll());
        const envs = s.split('\n');
        for (const k of envs) {
            if (k.length === 0 || !k.startsWith('export')) {
                continue;
            }
            const v = k.replace('export ', '').split('=');
            // TODO: "xx";
            tableView.model.appendRow({key: v[0], value: v[1]})
        }
        add.clicked();
        file.close();
    }
}
