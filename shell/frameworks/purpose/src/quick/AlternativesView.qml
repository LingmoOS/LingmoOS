/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.purpose

StackView {
    id: stack

    focus: true

    implicitHeight: currentItem.implicitHeight

    property bool running: false
    property alias pluginType: altsModel.pluginType
    property alias inputData: altsModel.inputData
    property Component highlight
    property Component header
    property Component footer
    property var verticalLayoutDirection: ListView.TopToBottom
    property Component delegate: Component {
        RowLayout {
            width: ListView.view.width
            Label {
                Layout.fillWidth: true
                text: display
                elide: Text.ElideRight
            }
            Button {
                text: i18nd("libpurpose6_quick", "Use")
                onClicked: createJob(index);
            }
            Keys.onReturnPressed: createJob(index)
            Keys.onEnterPressed: createJob(index)
        }
    }

    /**
     * Signals when the job finishes, reports the
     * @p error code and a text @p message.
     *
     * @p output will specify the output offered by the job
     */
    signal finished(var output, int error, string message)

    PurposeAlternativesModel {
        id: altsModel
    }

    /**
     * Adopts the job at the @p index.
     */
    function createJob(index) {
        stack.push(jobComponent, {index: index})
    }

    /**
     * Clears and returns back to the initial view.
     */
    function reset() {
        for(; stack.depth>1; stack.pop())
        {}
    }

    initialItem: ListView {
        ScrollBar.vertical: ScrollBar {}
        focus: true
        model: altsModel

        implicitHeight: contentHeight

        verticalLayoutDirection: stack.verticalLayoutDirection
        delegate: stack.delegate
        highlight: stack.highlight
        footer: stack.footer
        header: stack.header
    }

    Component {
        id: jobComponent

        JobView {
            id: jobView
            model: altsModel

            onStateChanged: {
                if (state === PurposeJobController.Finished || state === PurposeJobController.Error) {
                    stack.finished(jobView.job.output, jobView.job.error, jobView.job.errorString);
                } else if (state === PurposeJobController.Cancelled) {
                    stack.pop();
                }
            }

            Component.onCompleted: start()
        }
    }
}
