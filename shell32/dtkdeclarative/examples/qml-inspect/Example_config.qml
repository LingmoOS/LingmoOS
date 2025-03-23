// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0

Item {

    Config {
        id: exampleConfig
        name: "example"
        subpath: ""
        property int key2 : 1
        property string key3 : "key3 default"
        onKey3Changed: {
            key3Text.text = exampleConfig.key3
            console.info("Config key3Changed");
        }
    }
    Column {
        id: config
        spacing: 10
        Button{
            text: "generic function"
            onClicked: {
                console.info("configBtn pressed")
                console.info(exampleConfig.isValid())
                console.info(exampleConfig.value("key3", "ddd"))
                console.info(exampleConfig.value("key3"))
                console.info(exampleConfig.keyList())
                console.info(exampleConfig.name)
                console.info(exampleConfig.subpath)

                exampleConfig.setValue("key3", (new Date).getTime().toString())
                console.info(exampleConfig.value("key3", "ddd"))
            }
        }

        Row {
            spacing: 10

            Button{
                text: "write"
                onClicked: {
                    exampleConfig.key3 = (new Date).getTime().toString()
                }
            }

            Label {
                text: "property binding key3:" + exampleConfig.key3
            }

            Label {
                text: "js binding key3:"
            }
            Label {
                text: "exampleConfig.key3"
                Component.onCompleted: {
                    text = Qt.binding(function(){return exampleConfig.key3})
                }
            }
        }

        Row {
            spacing: 10

            Button{
                text: "read"
                onClicked: {
                    console.info("key3", exampleConfig.key3)
                    readKey3Text.text = exampleConfig.key3
                }
            }
            Label {
                text: "key3:"
            }
            Label {
                id: readKey3Text
            }

        }

        Row {
            spacing: 10

            Label {
                text: "Connections valueChanged:"
            }
            Label {
                text: "key3:"
            }
            Label {
                id: key3Text
                text: "key3:"
            }
            Connections{
                target: exampleConfig
                function onKey3Changed() {
                    key3Text.text = exampleConfig.key3
                    console.info("Connections key3Changed");
                }
            }
        }

    }
}
