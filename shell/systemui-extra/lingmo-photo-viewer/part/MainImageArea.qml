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

Item {
    id: container
    //位置和动画
    anchors.fill: parent
    anchors.leftMargin: 0
    Behavior on anchors.leftMargin { NumberAnimation { duration: variables.imageAniDuration } }
    anchors.bottomMargin: 0
    Behavior on anchors.bottomMargin { NumberAnimation { duration: variables.imageAniDuration } }
    anchors.topMargin: 0
    Behavior on anchors.topMargin { NumberAnimation { duration: variables.imageAniDuration } }
    anchors.rightMargin: 0

    property int imageType: 0 //判断动静图，需要加载哪一个qml
    property int itemIndex: 0

    signal zoomIn(var wheelDelta)
    signal zoomOut(var wheelDelta)
    signal zoomReset()
    signal zoomActual()
    signal rotate(var deg)
    signal rotateReset()
    signal mirrorH()
    signal mirrorV()
    signal mirrorReset()
    property string imageUrl:imageType == 0? "qrc:/res/res/kyview_logo.png" : "qrc:/res/res/loadgif.gif"

    MouseArea {
        id: transMouse
        anchors.fill: parent
        propagateComposedEvents:true
        onPositionChanged: {
            mouse.accepted = false
        }
        onDoubleClicked: {
            mouse.accepted = false
        }
        onClicked: {
            mouse.accepted = false
        }
        onReleased: {
            mouse.accepted = false
        }
    }

    Loader {
        id: imageloader
        objectName: "imageLoader"
        visible: true
        source: imageType == 0 ? "ImageNormal.qml" : "ImageAnimated.qml"
    }



}
