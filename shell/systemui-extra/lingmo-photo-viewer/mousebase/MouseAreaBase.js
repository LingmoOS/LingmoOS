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

function checkComboForShortcut(combo, wheelDelta) {

    if (wheelDelta.y > 0) {
        whatToDoWithFoundShortcut([0,"__zoomIn"], wheelDelta)
    } else {
        whatToDoWithFoundShortcut([0,"__zoomOut"], wheelDelta)
    }

}

function whatToDoWithFoundShortcut(sh, wheelDelta) {

    var cmd = sh[1]
    var obj
    for(var i=0; i<coverFLow.children.length; i++){
        if(coverFLow.children[i] instanceof PathView){
            for (var j = 0; j <coverFLow.children[i].currentItem.children.length;j++ ) {
                if (coverFLow.children[i].currentItem.children[j] instanceof Item) {
                    obj =coverFLow.children[i].currentItem.children[j]
                }
            }
        }
    }
    if(cmd === "__zoomIn") {
        obj.zoomIn(wheelDelta)
    } else if (cmd === "__zoomOut") {
        obj.zoomOut(wheelDelta)
    } else if(cmd === "__zoomReset") {
        obj.zoomReset()
    } else if(cmd === "__zoomActual") {
        obj.zoomActual()
    } else if(cmd === "__rotateL") {
        obj.rotate(-variables.angle90)
    } else if(cmd === "__rotateR") {
        obj.rotate(variables.angle90)
    } else if(cmd === "__rotate0") {
        obj.rotateReset()
    } else if(cmd === "__flipH") {
        obj.mirrorH()
    } else if(cmd === "__flipV") {
        obj.mirrorV()
    } else if(cmd === "__flipReset") {
        obj.mirrorReset()
    }

}
