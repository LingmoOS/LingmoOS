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

import QtQuick 2.12

Item {
    function recordCUrrentPOsAndScaleInfo(imageObj) {

        variables.historyScale = imageObj.curScale
        variables.historyX = imageObj.curX
        variables.historyY = imageObj.curY
    }

    //重置图片位置-居中
    function resetImagePostion(imageObj) {

        imageObj.curX = imageObj.parent.width/2 - imageObj.width/2
        imageObj.curY = imageObj.parent.height/2 - imageObj.height/2
        imageObj.x = Qt.binding(function() { return imageObj.curX })
        imageObj.y = Qt.binding(function() { return imageObj.curY })

        if (variables.operateImageWay === 7 || variables.operateImageWay === 0
                || variables.operateImageWay === 11 || variables.operateImageWay === 12
                || variables.operateImageWay === 17 || variables.operateImageWay === 18) {
            variables.origX = imageObj.curX
            variables.origY = imageObj.curY
        }
    }
    //首次打开； 同一张图片: 退出ocr，退出裁剪，适应窗口，原图展示，ocr；切图 更新缩放比
    function setImageScaleWithImageUpdate(imageObj,scaleValue) {
        imageObj.curScale = scaleValue
        if (variables.operateImageWay === 7 || variables.operateImageWay === 0
                || variables.operateImageWay === 11 || variables.operateImageWay === 12
                || variables.operateImageWay === 17 || variables.operateImageWay === 18) {
            variables.origScale = scaleValue
        }
    }
    //story 19807
    //还原自适应
    function imageRestore(imageObj) {
        var currentImagePath  = "image://main/" + variables.currentPath
        if (variables.imageListEveryAddType[variables.currentImageIndex]) {
            currentImagePath = "file://" + variables.currentPath
        }
        if (currentImagePath !== imageObj.source.toString()) {
            return
        }
        variables.doubleRestore = true
        if (variables.operateImageWay === 7 || variables.operateImageWay === 0
                || variables.operateImageWay === 11 || variables.operateImageWay === 12
                || variables.operateImageWay === 17 || variables.operateImageWay === 18
                || variables.operateImageWay === 3|| variables.operateImageWay === 4
                || variables.operateImageWay === 5|| variables.operateImageWay === 6) {
            if ((!variables.recordCurrentImageHasOperate && variables.historyScale == 0) || variables.origScale == variables.historyScale) {
                if (imageObj.curScale === variables.origScale) {
                    imageObj.curScale *= 2
                    imageObj.realCurX = mainWindow.width/2 - imageObj.width*imageObj.scale/2
                    imageObj.realCurY = mainWindow.height/2 - imageObj.height*imageObj.scale/2
                    resetImagePostion(imageObj)

                   imageActuralLeftUpPos(imageObj,imageObj.curX,imageObj.curY)
                   makeSureNavigatorNeedShow(imageObj)
                    imageObj.parent.sendScale(imageObj.curScale * 100)
                } else {
                    variables.recordCurrentImageHasOperate = false
                    variables.restoreOrigToToolbar = !variables.restoreOrigToToolbar
                    makeSureNavigatorNeedShow(imageObj)
                }

            } else {
                if (imageObj.curScale === variables.origScale) {
                    if (!variables.historyScale) {
                        imageObj.curScale *= 2
                        imageObj.realCurX = mainWindow.width/2 - imageObj.width*imageObj.scale/2
                        imageObj.realCurY = mainWindow.height/2 - imageObj.height*imageObj.scale/2
                        resetImagePostion(imageObj)
                        imageActuralLeftUpPos(imageObj,imageObj.curX,imageObj.curY)
                        makeSureNavigatorNeedShow(imageObj)
                        imageObj.parent.sendScale(imageObj.curScale * 100)
                    } else {
                        variables.restoreToHistory = true
                        imageObj.curX = variables.historyX
                        imageObj.curY = variables.historyY
                        imageObj.curScale = variables.historyScale
                        imageActuralLeftUpPos(imageObj,imageObj.curX,imageObj.curY)
                        makeSureNavigatorNeedShow(imageObj)
                        variables.restoreToHistory = false
                        variables.recordCurrentImageHasOperate = false
                        imageObj.parent.sendScale(imageObj.curScale * 100)

                    }
                } else {
                    variables.recordCurrentImageHasOperate = false
                    variables.restoreOrigToToolbar = !variables.restoreOrigToToolbar
                    makeSureNavigatorNeedShow(imageObj)
                }
            }

        } else {
            if (variables.recordCurrentImageHasOperate) {
                variables.recordCurrentImageHasOperate = false
                //发送适应窗口的信号给c++前端
                variables.restoreOrigToToolbar = !variables.restoreOrigToToolbar
            }
        }
    }

    //story 19807


    //返回图片的缩放比
    function getImageScale(imageObj) {
        return imageObj.curScale * variables.scalingconstant
    }

    //处理缩放等事件
    function performZoom(theImage, pos, wheelDelta, zoom_in, zoom_actual, zoom_pinch, zoom_pinchfactor) {

        if (variables.restoreToHistory) {
            pos = variables.histortPos
        }

        if (wheelDelta !== undefined) {
            if (wheelDelta.y > 12) {
                wheelDelta.y = 12
            } else if (wheelDelta.y < -12) {
                wheelDelta.y = -12
            }
        }
        // figure out zoom factor
        var zoomfactor

        // a PINCH occured
        if(zoom_pinch) {
            zoomfactor = zoom_pinchfactor
            // zoom to ACTUAL SIZE
        } else if (zoom_actual) {
            zoomfactor = 1/theImage.curScale
        } else {
            // zoom IN/OUT
            if (wheelDelta === undefined) {
                if (zoom_in) {
                    zoomfactor = Math.max(1.01, Math.min(1.3, 1 + variables.imageZoomSpeed*0.01))
                } else {
                    zoomfactor = 1/Math.max(1.01, Math.min(1.3, 1 + variables.imageZoomSpeed*0.01))
                }
            } else {
                if (zoom_in) {
                    zoomfactor = Math.max(1.01, Math.min(1.3, 1+Math.abs(wheelDelta.y/(101-variables.imageZoomSpeed))))
                } else {
                    zoomfactor = 1/Math.max(1.01, Math.min(1.3, 1+Math.abs(wheelDelta.y/(101-variables.imageZoomSpeed))))
                }
            }
        }

        var curZoomfactor = theImage.curScale * zoomfactor
        if (curZoomfactor > 100) {
            if (zoom_in || zoom_pinch) {
                zoomfactor = 100 / theImage.curScale
            }
        }
        if (curZoomfactor < 0.05) {
            if (!zoom_in) {
                zoomfactor = 0.05 / theImage.curScale
            }
        }
        if (variables.controlImageDoubleScale == true) {
            zoomfactor = 2 / theImage.curScale
        }

        // update x/y position of image
        var realX = (pos.x - theImage.width/2) * theImage.curScale
        var realY = (pos.y - theImage.height/2) * theImage.curScale

        // no rotation
        if(theImage.rotateTo % variables.angle360 == 0) {
            theImage.curX += (1-zoomfactor)*realX
            theImage.curY += (1-zoomfactor)*realY

            // rotated by 90 degrees
        } else if(theImage.rotateTo % variables.angle360 == variables.angle90 || theImage.rotateTo % variables.angle360 == -variables.angle270) {

            theImage.curX -= (1-zoomfactor)*realY
            theImage.curY += (1-zoomfactor)*realX

            // rotated by 180 degrees
        } else if(Math.abs(theImage.rotateTo % variables.angle360) == variables.angle180) {

            theImage.curX -= (1-zoomfactor)*realX
            theImage.curY -= (1-zoomfactor)*realY

            // rotated by 270 degrees
        } else if(theImage.rotateTo%variables.angle360 == variables.angle270 || theImage.rotateTo%variables.angle360 == -variables.angle90) {

            theImage.curX += (1-zoomfactor)*realY
            theImage.curY -= (1-zoomfactor)*realX

        } else {
            console.log("ERROR: unknown rotation step:", theImage.rotateTo)
        }

        var curScale = theImage.curScale * zoomfactor
        theImage.curScale = curScale
        theImage.parent.sendScale(theImage.curScale * 100)
        imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
    }

    //返回图片左上角可视区域的点在缩放的图片的点的位置
    function imageActuralLeftUpPos(theImage,curX,curY) {

        var theImageCurX = curX
        var theImageCurY = curY

        var actualY
        var actualX
        if (theImageCurY >= 0) {
            actualY = theImageCurY + theImage.height / 2 - (theImage.height * theImage.curScale) / 2
        } else {
            actualY =  theImage.height / 2  -  Math.abs(theImageCurY) - theImage.height / 2 * theImage.curScale
        }
        if (theImageCurX >= 0) {
            actualX = theImageCurX + theImage.width / 2 - (theImage.width * theImage.curScale) / 2
        } else {
            actualX = theImage.width / 2  -  Math.abs(theImageCurX) - theImage.width / 2 * theImage.curScale
        }
        if(Math.abs(theImage.rotateTo % variables.angle360) == variables.angle90 || Math.abs(theImage.rotateTo % variables.angle360) == variables.angle270) {
            actualX = actualX + theImage.width * theImage.curScale / 2 - theImage.height * theImage.curScale / 2
            actualY = actualY + theImage.height * theImage.curScale / 2 - theImage.width * theImage.curScale /2
        }
        theImage.parent.imageLeftUpX = actualX
        theImage.parent.imageLeftUpY = actualY
        getLeftUpPosAccordingActualImage(actualX,actualY,theImage)
    }

    function getLeftUpPosAccordingActualImage(currLeftUpX,currLeftUpY,theImage) {

        var actualX = currLeftUpX
        var actualY = currLeftUpY

        if (actualX >= 0) {
            theImage.parent.imageVisibleLeftUpX = 0
        } else {
            theImage.parent.imageVisibleLeftUpX = -actualX
        }
        if (actualY >= 0) {
            theImage.parent.imageVisibleLeftUpY = 0
        } else {
            theImage.parent.imageVisibleLeftUpY = -actualY
        }

        //        makeSureNavigatorNeedShow(theImage)
    }
    function makeSureNavigatorNeedShow(theImage) {

        var visibleW
        var visibleH

        if(Math.abs(theImage.rotateTo % variables.angle360) == variables.angle90 || Math.abs(theImage.rotateTo % variables.angle360) == variables.angle270) {
            visibleH = theImage.width * theImage.curScale
            visibleW =  theImage.height * theImage.curScale
        } else {
            visibleW = theImage.width * theImage.curScale
            visibleH =  theImage.height * theImage.curScale
        }

        if (visibleW <= variables.curWidW && visibleH <= variables.curWidH) {
            theImage.parent.mouseareaEnabeled = false
            variables.navigatorState = false
            theImage.parent.closeNavigator()
            theImage.parent.imageNeedCenterZoom = true
            return
        }
        //超出窗口大小需要显示导航栏
        if (theImage.parent.imageLeftUpX < 0 || theImage.parent.imageLeftUpY < 0 || theImage.parent.imageLeftUpX + visibleW > variables.curWidW || theImage.parent.imageLeftUpY + visibleH > variables.curWidH) {
            theImage.parent.mouseareaEnabeled = true
            variables.navigatorState = true
            theImage.parent.imageNeedCenterZoom = false
            theImage.parent.needShowNavigator(theImage.curScale,theImage.parent.imageVisibleLeftUpX,theImage.parent.imageVisibleLeftUpY,getVisibleSizeAccordingWidW(theImage),getVisibleSizeAccordingWidH(theImage))
        }
    }
    //返回所需要相对于窗口的在当前缩放比下的可视区域的高度和宽度
    function getVisibleSizeAccordingWidW(theImage) {

        if (variables.curWidW === 0) {
            return 1
        }

        var visibleSizeAccordingWidW;
        if(Math.abs(theImage.rotateTo % variables.angle360) == variables.angle90 || Math.abs(theImage.rotateTo % variables.angle360) == variables.angle270) {
            if (theImage.parent.imageLeftUpX >= 0) {
                if (variables.curWidW - theImage.parent.imageLeftUpX - theImage.height * theImage.curScale <=0) {
                    visibleSizeAccordingWidW =  variables.curWidW - theImage.parent.imageLeftUpX
                } else {
                    visibleSizeAccordingWidW = theImage.height * theImage.curScale
                }
            } else {
                if (variables.curWidW - (theImage.height * theImage.curScale + theImage.parent.imageLeftUpX) >=0)  {
                    visibleSizeAccordingWidW = theImage.height * theImage.curScale + theImage.parent.imageLeftUpX
                } else {
                    visibleSizeAccordingWidW = variables.curWidW
                }
            }
        } else {
            if (theImage.parent.imageLeftUpX >= 0) {
                if (variables.curWidW - theImage.parent.imageLeftUpX - theImage.width * theImage.curScale <=0) {
                    visibleSizeAccordingWidW =  variables.curWidW - theImage.parent.imageLeftUpX
                } else {
                    visibleSizeAccordingWidW = theImage.width * theImage.curScale
                }

            } else {
                if (variables.curWidW - (theImage.width * theImage.curScale + theImage.parent.imageLeftUpX) >=0)  {
                    visibleSizeAccordingWidW = theImage.width * theImage.curScale + theImage.parent.imageLeftUpX
                } else {
                    visibleSizeAccordingWidW = variables.curWidW
                }
            }
        }
        return visibleSizeAccordingWidW

    }
    function getVisibleSizeAccordingWidH(theImage) {

        if (variables.curWidH === 0) {
            return 1
        }

        var visibleSizeAccordingWidH;
        if(Math.abs(theImage.rotateTo % variables.angle360) == variables.angle90 || Math.abs(theImage.rotateTo % variables.angle360) == variables.angle270) {
            if (theImage.parent.imageLeftUpY >= 0) {
                if (variables.curWidH - theImage.parent.imageLeftUpY - theImage.width * theImage.curScale <=0) {
                    visibleSizeAccordingWidH =  variables.curWidH - theImage.parent.imageLeftUpY
                } else {
                    visibleSizeAccordingWidH = theImage.width * theImage.curScale
                }
            } else {
                if (variables.curWidH - (theImage.width * theImage.curScale + theImage.parent.imageLeftUpY) >=0)  {
                    visibleSizeAccordingWidH = theImage.width * theImage.curScale + theImage.parent.imageLeftUpY
                } else {
                    visibleSizeAccordingWidH = variables.curWidH
                }
            }
        } else {
            if (theImage.parent.imageLeftUpY >= 0) {
                if (variables.curWidH - theImage.parent.imageLeftUpY - theImage.height * theImage.curScale <=0) {
                    visibleSizeAccordingWidH =  variables.curWidH - theImage.parent.imageLeftUpY
                } else {
                    visibleSizeAccordingWidH = theImage.height * theImage.curScale
                }
            } else {
                if (variables.curWidH - (theImage.height * theImage.curScale + theImage.parent.imageLeftUpY) >=0)  {
                    visibleSizeAccordingWidH = theImage.height * theImage.curScale + theImage.parent.imageLeftUpY
                } else {
                    visibleSizeAccordingWidH = variables.curWidH
                }
            }
        }
        return visibleSizeAccordingWidH
    }


    //返回所需要相对于窗口的X和Y
    function getLeftUpPosAccordingWidX(theImage) {

        var leftUpPosAccordingWidX;
        if (theImage.parent.imageLeftUpX > 0) {
            leftUpPosAccordingWidX = theImage.parent.imageLeftUpX
        } else {
            leftUpPosAccordingWidX = 0
        }
        return leftUpPosAccordingWidX

    }
    function getLeftUpPosAccordingWidY(theImage) {

        var leftUpPosAccordingWidY;
        if (theImage.parent.imageLeftUpY > 0) {
            leftUpPosAccordingWidY = theImage.parent.imageLeftUpY
        } else {
            leftUpPosAccordingWidY = 0
        }
        return leftUpPosAccordingWidY
    }


    function operateNavigatorChangeImagePos(leftUpPoint,theImage) {

        var imageX = theImage.curX
        var imageY = theImage.curY
        imageX -= leftUpPoint.x
        imageY -= leftUpPoint.y
        theImage.curX = imageX
        theImage.curY = imageY
        imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        makeSureNavigatorNeedShow(theImage)
    }


    function controlImageInWid(theImage) {

        if(Math.abs(theImage.rotateTo % variables.angle360) == variables.angle90 || Math.abs(theImage.rotateTo % variables.angle360) == variables.angle270) {

            if (theImage.parent.mouseDragType === 0) {
                if (theImage.parent.imageLeftUpX > 0) {
                    theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                } else {
                    if (variables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        theImage.curX = theImage.curX + variables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                    }
                    theImage.curY = theImage.parent.height/2 - theImage.height/2
                }

            } else if (theImage.parent.mouseDragType === 1) {
                if (theImage.parent.imageLeftUpY > 0) {
                    theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                } else {
                    if (variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                        theImage.curY = theImage.curY + variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                    }
                    theImage.curX = theImage.parent.width/2 - theImage.width/2
                }
            } else if (theImage.parent.mouseDragType === 2) {

                if (theImage.parent.imageLeftUpX < 0 && theImage.parent.imageLeftUpY < 0) {

                    if (variables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        if (variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curX = theImage.curX + variables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                            theImage.curY = theImage.curY + variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        } else {
                            theImage.curX = theImage.curX + variables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                        }
                    } else {
                        if (variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curY = theImage.curY + variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        }
                    }
                } else if  (theImage.parent.imageLeftUpX < 0 && theImage.parent.imageLeftUpY > 0) {

                    if (variables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        if (theImage.parent.imageVisibleLeftUpY < variables.curWidH) {
                            theImage.curX = theImage.curX + variables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        } else {
                            theImage.curX = theImage.curX + variables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                        }
                    } else {
                        if (theImage.parent.imageVisibleLeftUpY < variables.curWidH) {
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        }
                    }

                } else if  (theImage.parent.imageLeftUpX > 0 && theImage.parent.imageLeftUpY < 0) {

                    if (theImage.parent.imageVisibleLeftUpX < variables.curWidW) {
                        if ( variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                            theImage.curY = theImage.curY + variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        } else {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                        }
                    } else {
                        if ( variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curY = theImage.curY + variables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        }
                    }

                } else if (theImage.parent.imageLeftUpX > 0 && theImage.parent.imageLeftUpY > 0) {
                    if (theImage.parent.imageVisibleLeftUpX < variables.curWidW) {
                        if ( theImage.parent.imageVisibleLeftUpY < variables.curWidH) {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        } else {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                        }
                    } else {
                        if ( theImage.parent.imageVisibleLeftUpY < variables.curWidH) {
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        }
                    }
                }
            } else {
                resetImagePostion(theImage)
            }
        } else {

            if (theImage.parent.mouseDragType === 0) {
                if (theImage.parent.imageLeftUpX > 0) {
                    theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                } else {
                    if (variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        theImage.curX = theImage.curX + variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                    }
                    theImage.curY = theImage.parent.height/2 - theImage.height/2
                }
            } else if (theImage.parent.mouseDragType === 1) {
                if (theImage.parent.imageLeftUpY > 0) {
                    theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                } else {
                    if (variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                        theImage.curY = theImage.curY + variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                    }
                    theImage.curX = theImage.parent.width/2 - theImage.width/2

                }
            } else if (theImage.parent.mouseDragType === 2) {

                if (theImage.parent.imageLeftUpX < 0 && theImage.parent.imageLeftUpY < 0 && (variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0 || variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0)) {

                    if (variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        if (variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curX = theImage.curX + variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                            theImage.curY = theImage.curY + variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        } else {
                            theImage.curX = theImage.curX + variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                        }
                    } else {
                        if (variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curY = theImage.curY + variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        }
                    }

                } else if  (theImage.parent.imageLeftUpX < 0 && theImage.parent.imageLeftUpY > 0) {

                    if (variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        if (theImage.parent.imageVisibleLeftUpY < variables.curWidH) {
                            theImage.curX = theImage.curX + variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        } else {
                            theImage.curX = theImage.curX + variables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                        }
                    } else {
                        if (theImage.parent.imageVisibleLeftUpY < variables.curWidH) {
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        }
                    }

                } else if  (theImage.parent.imageLeftUpX > 0 && theImage.parent.imageLeftUpY < 0) {
                    if (theImage.parent.imageVisibleLeftUpX < variables.curWidW) {
                        if ( variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                            theImage.curY = theImage.curY + variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        } else {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                        }
                    } else {
                        if ( variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curY = theImage.curY + variables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        }
                    }

                } else if (theImage.parent.imageLeftUpX > 0 && theImage.parent.imageLeftUpY > 0) {

                    if (theImage.parent.imageVisibleLeftUpX < variables.curWidW) {
                        if ( theImage.parent.imageVisibleLeftUpY < variables.curWidH) {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        } else {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                        }
                    } else {
                        if ( theImage.parent.imageVisibleLeftUpY < variables.curWidH) {
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        }
                    }
                }
            } else {
                resetImagePostion(theImage)
            }
        }
    }

    function resetSignArgs() {

        variables.painterType = -1                 //记录画笔类型
        variables.painterThickness = -1             //记录画笔粗细
        variables.painterColor = -1                 //记录画笔颜色
        variables.isBold =false                 //加粗
        variables.isDeleteLine = false             //删除
        variables.isUnderLine =false             //下划线
        variables.isItalics =false                //斜体
        variables.fontType = ""                //字体
        variables.fontSize = 6                      //字号

        variables.objList = []
        variables.operateTime = 0

        variables.mouseqX = 0
        variables.mouseqY = 0
    }

}
