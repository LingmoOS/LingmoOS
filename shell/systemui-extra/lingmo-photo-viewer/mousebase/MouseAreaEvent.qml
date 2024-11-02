/**************************************************************************
 **                                                                      **
 ** Copyright (C) 2011-2021 Lukas Spies                                  **
 ** Contact: http://photoqt.org                                          **
 **                                                                      **
 ** This file is part of PhotoQt.                                        **
 **                                                                      **
 ** PhotoQt is free software: you can redistribute it and/or modify      **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation, either version 2 of the License, or    **
 ** (at your option) any later version.                                  **
 **                                                                      **
 ** PhotoQt is distributed in the hope that it will be useful,           **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with PhotoQt. If not, see <http://www.gnu.org/licenses/>.      **
 **                                                                      **
 **************************************************************************/

import QtQuick 2.9

import "./MouseAreaBase.js" as MouseAreaBase

Item {
    anchors.fill: parent
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton|Qt.RightButton|Qt.MiddleButton
        hoverEnabled: true
        cursorShape:  Qt.ArrowCursor

        property int angleDeltaX: 0
        property int angleDeltaY: 0


        onWheel: {

            var combo = ""

            angleDeltaX += wheel.angleDelta.x
            angleDeltaY += wheel.angleDelta.y

            variables.wheelVaule = Qt.point(angleDeltaX, angleDeltaY)
            MouseAreaBase.checkComboForShortcut(combo, Qt.point(angleDeltaX, angleDeltaY))

            angleDeltaX = 0
            angleDeltaY = 0
            if (variables.operateImageWay != 9 && variables.operateImageWay != 10) {
               variables.wheelChange = !variables.wheelChange
            }

        }
    }

}
