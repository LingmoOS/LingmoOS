/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

// ButtonsPage

LingmoComponents.Page {
    id: editorPage

    property string shader
    property alias shaderText: editor.text
    property string pageName: "Editor"
    property string icon: "accessories-text-editor"

    anchors {
        fill: parent
        margins: _s
    }

    Image {
        id: imageItem
        anchors.fill: parent
        //source: "../images/elarun-small.png"
    }

    ShaderEffectSource {
        id: effectSource
        sourceItem: imageItem
        //hideSource: hideSourceCheckbox.checked
        hideSource: true
    }

    ShaderEffect {
        id: mainShader
        anchors.fill: editorPage
        property ShaderEffectSource source: effectSource
        property real f: 0
        property real f2: 0
        property int intensity: 1
        smooth: true
    }
    LingmoComponents.ToolButton {
        iconSource: "dialog-close"
        width: _h
        height: width
        visible: !(mainShader.fragmentShader == "" && mainShader.vertexShader == "")
        anchors { top: parent.top; right: parent.right; }
        onClicked: {
            mainShader.fragmentShader = "";
            mainShader.vertexShader = "";
            editorPage.shader = ""
            vertexPage.shader = ""
        }
    }


    LingmoUI.Heading {
        id: heading
        level: 1
        anchors {
            top: parent.top;
            left: parent.left
            right: parent.right
        }
        text: pageName
    }
    LingmoComponents.ButtonColumn {
        anchors {
            right: parent.right
            top: heading.top
        }
        LingmoComponents.RadioButton {
            id: fragmentRadio
            text: "Fragment / Pixel Shader"
        }
        LingmoComponents.RadioButton {
            text: "Vertex Shader"
        }
    }

//     LingmoComponents.TextArea {
//         id: editor
//         anchors {
//             top: heading.bottom;
//             topMargin: _s
//             left: parent.left
//             right: parent.right
//             bottom: applyButton.top
//             wrapMode: TextEdit.Wrap
//             bottomMargin: _s
//
//         }
// //         text: { "void main(void) {\
// //         gl_FragColor = vec4(1.0, 0.0, 0.0, 0.3);\
// //     }"
// //         }
//         text:"
//         void main(void) {
//             gl_FragColor = vec4(0.2, 0.8, 0.6, 0.3);
//         }
//         "
//
// //         width: parent.width
// //         parent.height-height: _h*2
//     }

    LingmoComponents.Button {
        id: applyButton
        text: "Upload Shader"
        onClicked: {
            shader = editor.text
            if (fragmentRadio.checked) {
                print("Uploading new fragment shader: \n" + shader);
                mainShader.fragmentShader = shader
            } else {
                print("Uploading new vertex shader: \n" + shader);
                mainShader.vertexShader = shader;
            }
        }

        anchors {
            right: parent.right
            bottom: parent.bottom

        }



    }
//     LingmoComponents.CheckBox {
//         id: hideSourceCheckbox
//         text: "Hide Source Item"
//         anchors { bottom: parent.bottom; left: parent.left; margins: _s; }
//         onCheckedChanged: effectSource.hideSource = checked
//     }

}
