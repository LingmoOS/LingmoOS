/*
* Copyright 2022 KylinSoft Co., Ltd.
*
* This program is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import QtQuick.Controls 2.0

Rectangle {
    width: parent.width
    height: virtualKeyboard.toolAndCandidateHeight
    color: virtualKeyboard.virtualKeyboardColor
    visible: !virtualKeyboard.isToolbarVisible

    Rectangle {
        id: candidateListRectangle
        width: virtualKeyboard.candidateListWidth
        height: virtualKeyboard.toolAndCandidateHeight - virtualKeyboard.cardinalNumber
        color: virtualKeyboard.candidateListBackgroundColor
        anchors.horizontalCenter: parent.horizontalCenter
        y: cardinalNumber
        radius: 15
        ListView {
            x: virtualKeyboard.candidateListX
            width: virtualKeyboard.candidateListWidth - x
            height: parent.height
            visible: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            orientation: ListView.Horizontal
            model: virtualKeyboard.candidateList
            spacing: 20
            currentIndex: virtualKeyboard.globalCursorIndex
            delegate: Text {
                id: candidate
                text: modelData
                color: ListView.isCurrentItem
                ? virtualKeyboard.candidateHighlightColor
                : virtualKeyboard.candidateDefaultColor
                font.pointSize: virtualKeyboard.candidateFontSize
                // TODO(litian): 使用更好的方式使候选词居中
                /*设置居中，设置anchors.verticalCenter: parent.verticalCenter会报错
                TypeError: Cannot read property 'verticalCenter' of null, 因此手动计算居中位置*/
                y: (candidateListRectangle.height - height)/2
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        virtualKeyboard.selectCandidate(index)
                        console.debug("点击了", index, modelData)
                    }
                }
            }
        }
    }

}

