// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Layouts 1.7
import org.deepin.dtk 1.0
import org.deepin.tweak 1.0

Frame {
    property string displayName: "Test Name"
    property string description: "Test description"
    property string version: "1.0.0"
    property string author: "justforlxz"
    property string icon: "icon.png"

    ColumnLayout {
        Text {
            text: "I'm qml loader content"
        }
        RowLayout {
            Text {
                id: label
                text: "Hello: "
            }
            LineEdit {
            }
            Button {
                id: button
                onClicked: {
                    let vv = Tweak.Launcher().program("ls").arguments("/");
                    console.log("sync =====");
                    let sync = vv.call();
                    console.log("standard output: \n", sync.allStandardOutput);
                    console.log("standard error: \n", sync.allStandardError);
                    console.log('async ======');
                    vv.asyncCall((callback) => {
                      console.log("standard output: \n", callback.allStandardOutput);
                      console.log("standard error: \n", callback.allStandardError);
                      Tweak.Launcher().program("ls").arguments("/").call()
                    });
                    let file = Tweak.File("/tmp/test.file");
                    console.log(typeof FileMode)
                    console.log(typeof FileMode.ReadOnly);
                    let open = file.open(FileMode.ReadOnly);
                    if (open) {
                      console.log('===== ready read');
                      console.log(file.readAll());
                      console.log('===== read all.');
                    }
                    console.log('file exist: ', Tweak.File('/tmp/test.file').exists());
                  }
              }
        }
        Image {
            id: img

            source: "tile.png"
            sourceSize.width: 300
            sourceSize.height: 300
        }
    }
}
