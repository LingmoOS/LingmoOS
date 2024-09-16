// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Live Block 的控制脚本，移植自Qt的帮助文档

var component;
var block;
var parent

function createBlockObjects(parent) {
    component = Qt.createComponent("LiveBlock.qml");
    if (component.status === Component.Ready) {
        finishCreation(parent);
    } else {
        component.statusChanged.connect(finishCreation);
    }
}

function finishCreation(parent) {
    if (component.status === Component.Ready) {
        block = component.createObject(parent);
        if (block === null) {
            // Error Handling
            console.log("Error creating object");
        }
    } else if (component.status === Component.Error) {
        // Error Handling
        console.log("Error loading component:", component.errorString());
    }
}
