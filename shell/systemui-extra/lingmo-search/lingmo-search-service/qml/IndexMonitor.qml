/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12

Rectangle {
    height: 250
    width: 320
    Column {
        visible: true
        width: parent.width
        height: childrenRect.height
        StatusKeyValue {
            id: basicIndexDocumentNum
            keyText: qsTr("Basic index document number")
            valueText: monitor.basicIndexDocNum
        }

        StatusKeyValue {
            id: contentIndexDocumentNum
            keyText: qsTr("Content index document number")
            valueText: monitor.contentIndexDocNum
        }
        StatusKeyValue {
            id: ocrContentIndexDocumentNum
            keyText: qsTr("OCR content index document number")
            valueText: monitor.ocrContentIndexDocNum
        }
        StatusKeyValue {
            id: aiIndexDocumentNum
            keyText: qsTr("ai index document number")
            valueText: monitor.aiIndexDocNum
        }

        StatusKeyValue {
            id: indexState
            keyText: qsTr("Index state")
            valueText: monitor.indexState
        }

        IndexProgressBar {
            id: basicIndexProgress
            width: parent.width;
            name: "Basic index progress"
            from: 0
            to: monitor.basicIndexSize
            value: monitor.basicIndexProgress
            visible: true
        }

        IndexProgressBar {
            id: contentIndexProgress
            width: parent.width;
            name: "Content index progress"
            from: 0
            to: monitor.contentIndexSize
            value: monitor.contentIndexProgress
            visible: true
        }

        IndexProgressBar {
            id: ocrContentIndexProgress
            name: "OCR index progress"
            width: parent.width;
            from: 0
            to: monitor.ocrContentIndexSize
            value: monitor.ocrContentIndexProgress
            visible: true
        }
        IndexProgressBar {
            id: aiIndexProgress
            name: "Ai progress"
            width: parent.width;
            from: 0
            to: monitor.aiIndexSize
            value: monitor.aiIndexProgress
            visible: true
        }
    }
    Component.onCompleted: {
        monitor.basicIndexDone.connect(onBasicIndexDone);
        monitor.contentIndexDone.connect(onContentIndexDone);
        monitor.ocrContentIndexDone.connect(onOcrContentIndexDone);
        monitor.aiIndexDone.connect(onAiIndexDone);
        monitor.basicIndexStart.connect(onBasicIndexStart);
        monitor.contentIndexStart.connect(onContentIndexStart);
        monitor.ocrContentIndexStart.connect(onOcrContentIndexStart);
        monitor.aiIndexStart.connect(onAiIndexStart);
    }
    function onBasicIndexStart() {
        basicIndexProgress.state = "Running"
    }
    function onContentIndexStart() {
        contentIndexProgress.state = "Running"
    }
    function onOcrContentIndexStart() {
        ocrContentIndexProgress.state = "Running"
    }
    function onAiIndexStart() {
        aiIndexProgress.state = "Running"
    }
    function onBasicIndexDone() {
        basicIndexProgress.state = "Done"
    }
    function onContentIndexDone() {
        contentIndexProgress.state = "Done"
    }
    function onOcrContentIndexDone() {
        ocrContentIndexProgress.state = "Done"
    }
    function onAiIndexDone() {
        aiIndexProgress.state = "Done"
    }
}
