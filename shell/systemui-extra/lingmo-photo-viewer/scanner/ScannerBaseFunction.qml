import QtQuick 2.12

Item {
    //重置图片位置-居中
    function resetImagePostion(imageObj) {
        imageObj.curX = imageObj.parent.width/2 - imageObj.width/2
        imageObj.curY = imageObj.parent.height/2 - imageObj.height/2
        imageObj.x = Qt.binding(function() { return imageObj.curX })
        imageObj.y = Qt.binding(function() { return imageObj.curY })
    }
    //图片更新设置图片缩放比
    function setImageScaleWithImageUpdate(imageObj,scaleValue) {
        imageObj.curScale = scaleValue

    }
    //返回图片的缩放比
    function getImageScale(imageObj) {
        return imageObj.curScale * scannerVariables.scalingconstant
    }
    //处理缩放
    function performZoom(theImage, pos, wheelDelta, zoom_in, zoom_actual, zoom_pinch, zoom_pinchfactor) {
        if (wheelDelta !== undefined) {
            if (wheelDelta.y > 12) {
                wheelDelta.y = 12
            } else if (wheelDelta.y < -12) {
                wheelDelta.y = -12
            }
        }
        //缩放系数
        var zoomfactor
        if(zoom_pinch) {
            zoomfactor = zoom_pinchfactor
        } else if (zoom_actual) {
            zoomfactor = 1/theImage.curScale
        } else {
            if (wheelDelta === undefined) {
                if (zoom_in) {
                    zoomfactor = Math.max(1.01, Math.min(1.3, 1 + scannerVariables.imageZoomSpeed*0.01))
                } else {
                    zoomfactor = 1/Math.max(1.01, Math.min(1.3, 1 + scannerVariables.imageZoomSpeed*0.01))
                }
            } else {
                if (zoom_in) {
                    zoomfactor = Math.max(1.01, Math.min(1.3, 1+Math.abs(wheelDelta.y/(101-scannerVariables.imageZoomSpeed))))
                } else {
                    zoomfactor = 1/Math.max(1.01, Math.min(1.3, 1+Math.abs(wheelDelta.y/(101-scannerVariables.imageZoomSpeed))))
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
        // update x/y position of image
        var realX = (pos.x - theImage.width/2) * theImage.curScale
        var realY = (pos.y - theImage.height/2) * theImage.curScale
        // no rotation
        if(theImage.rotateTo % scannerVariables.angle360 == 0) {
            theImage.curX += (1-zoomfactor)*realX
            theImage.curY += (1-zoomfactor)*realY

            // rotated by 90 degrees
        } else if(theImage.rotateTo % scannerVariables.angle360 == scannerVariables.angle90 || theImage.rotateTo % scannerVariables.angle360 == -scannerVariables.angle270) {

            theImage.curX -= (1-zoomfactor)*realY
            theImage.curY += (1-zoomfactor)*realX

            // rotated by 180 degrees
        } else if(Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle180) {

            theImage.curX -= (1-zoomfactor)*realX
            theImage.curY -= (1-zoomfactor)*realY

            // rotated by 270 degrees
        } else if(theImage.rotateTo%scannerVariables.angle360 == scannerVariables.angle270 || theImage.rotateTo%scannerVariables.angle360 == -scannerVariables.angle90) {

            theImage.curX += (1-zoomfactor)*realY
            theImage.curY -= (1-zoomfactor)*realX

        } else {
            console.log("ERROR: unknown rotation step:", theImage.rotateTo)
        }

        var curScale = theImage.curScale * zoomfactor
        theImage.curScale = curScale
        theImage.parent.sendScale(theImage.curScale * 100)
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
        if(Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 || Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270) {
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
    }
    //判断图片是否超出窗口大小
    function makeSureImageOverWid(theImage) {
        var visibleW
        var visibleH

        if(Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 || Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270) {
            visibleH = theImage.width * theImage.curScale
            visibleW =  theImage.height * theImage.curScale
        } else {
            visibleW = theImage.width * theImage.curScale
            visibleH =  theImage.height * theImage.curScale
        }

        if (visibleW <= scannerVariables.curWidW && visibleH <= scannerVariables.curWidH) {
            theImage.parent.imageNeedCenterZoom = true
            return
        }
        //超出窗口大小
        if (theImage.parent.imageLeftUpX < 0 || theImage.parent.imageLeftUpY < 0 || theImage.parent.imageLeftUpX + visibleW > scannerVariables.curWidW || theImage.parent.imageLeftUpY + visibleH > scannerVariables.curWidH) {
            theImage.parent.imageNeedCenterZoom = false
        }
    }
    //返回所需要相对于窗口的在当前缩放比下的可视区域的宽度
    function getScannerVisibleSizeAccordingWidW(theImage) {

        if (scannerVariables.curWidW === 0) {
            return 1
        }
        var visibleSizeAccordingWidW;
        if(Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 || Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270) {
            if (theImage.parent.imageLeftUpX >= 0) {
                if (scannerVariables.curWidW - theImage.parent.imageLeftUpX - theImage.height * theImage.curScale <=0) {
                    visibleSizeAccordingWidW =  scannerVariables.curWidW - theImage.parent.imageLeftUpX
                } else {
                    visibleSizeAccordingWidW = theImage.height * theImage.curScale
                }
            } else {
                if (scannerVariables.curWidW - (theImage.height * theImage.curScale + theImage.parent.imageLeftUpX) >=0)  {
                    visibleSizeAccordingWidW = theImage.height * theImage.curScale + theImage.parent.imageLeftUpX
                } else {
                    visibleSizeAccordingWidW = scannerVariables.curWidW
                }
            }
        } else {
            //缩放比1且高宽小于主界面的，无需计算，直接返回图片尺寸
            if (theImage.curScale == 1 && theImage.width <= scannerVariables.curWidW) {
                visibleSizeAccordingWidW =  theImage.width
                return visibleSizeAccordingWidW
            }
            if (theImage.parent.imageLeftUpX >= 0) {
                if (scannerVariables.curWidW - theImage.parent.imageLeftUpX - theImage.width * theImage.curScale <=0) {
                    visibleSizeAccordingWidW =  scannerVariables.curWidW - theImage.parent.imageLeftUpX
                } else {
                    visibleSizeAccordingWidW = theImage.width * theImage.curScale
                }

            } else {
                if (scannerVariables.curWidW - (theImage.width * theImage.curScale + theImage.parent.imageLeftUpX) >=0)  {
                    visibleSizeAccordingWidW = theImage.width * theImage.curScale + theImage.parent.imageLeftUpX
                } else {
                    visibleSizeAccordingWidW = scannerVariables.curWidW
                }
            }
        }
        return visibleSizeAccordingWidW

    }
    //返回所需要相对于窗口的在当前缩放比下的可视区域的高度
    function getScannerVisibleSizeAccordingWidH(theImage) {
        if (scannerVariables.curWidH === 0) {
            return 1
        }

        var visibleSizeAccordingWidH;
        if(Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 || Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270) {
            if (theImage.parent.imageLeftUpY >= 0) {
                if (scannerVariables.curWidH - theImage.parent.imageLeftUpY - theImage.width * theImage.curScale <=0) {
                    visibleSizeAccordingWidH =  v.curWidH - theImage.parent.imageLeftUpY
                } else {
                    visibleSizeAccordingWidH = theImage.width * theImage.curScale
                }
            } else {
                if (scannerVariables.curWidH - (theImage.width * theImage.curScale + theImage.parent.imageLeftUpY) >=0)  {
                    visibleSizeAccordingWidH = theImage.width * theImage.curScale + theImage.parent.imageLeftUpY
                } else {
                    visibleSizeAccordingWidH = scannerVariables.curWidH
                }
            }
        } else {
            //缩放比1且高宽小于主界面的，无需计算，直接返回图片尺寸
            if (theImage.curScale == 1 && theImage.height <= scannerVariables.curWidH) {
                visibleSizeAccordingWidH =  theImage.height
                return visibleSizeAccordingWidH
            }

            if (theImage.parent.imageLeftUpY >= 0) {
                if (scannerVariables.curWidH - theImage.parent.imageLeftUpY - theImage.height * theImage.curScale <=0) {
                    visibleSizeAccordingWidH =  scannerVariables.curWidH - theImage.parent.imageLeftUpY
                } else {
                    visibleSizeAccordingWidH = theImage.height * theImage.curScale
                }
            } else {
                if (scannerVariables.curWidH - (theImage.height * theImage.curScale + theImage.parent.imageLeftUpY) >=0)  {
                    visibleSizeAccordingWidH = theImage.height * theImage.curScale + theImage.parent.imageLeftUpY
                } else {
                    visibleSizeAccordingWidH = scannerVariables.curWidH
                }
            }
        }
        return visibleSizeAccordingWidH
    }
    //返回所需要相对于窗口的X
    function getScannerLeftUpPosAccordingWidX(theImage) {
        var leftUpPosAccordingWidX;
        if (theImage.parent.imageLeftUpX > 0) {
            leftUpPosAccordingWidX = theImage.parent.imageLeftUpX
        } else {
            leftUpPosAccordingWidX = 0
        }
        return leftUpPosAccordingWidX

    }
    //返回所需要相对于窗口的Y
    function getScannerLeftUpPosAccordingWidY(theImage) {
        var leftUpPosAccordingWidY;
        if (theImage.parent.imageLeftUpY > 0) {
            leftUpPosAccordingWidY = theImage.parent.imageLeftUpY
        } else {
            leftUpPosAccordingWidY = 0
        }
        return leftUpPosAccordingWidY
    }
    function controlImageInWid(theImage) {

        if(Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle90 || Math.abs(theImage.rotateTo % scannerVariables.angle360) == scannerVariables.angle270) {

            if (theImage.parent.mouseDragType === 0) {
                if (theImage.parent.imageLeftUpX > 0) {
                    theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                } else {
                    if (scannerVariables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                    }
                    theImage.curY = theImage.parent.height/2 - theImage.height/2
                }

            } else if (theImage.parent.mouseDragType === 1) {
                if (theImage.parent.imageLeftUpY > 0) {
                    theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                } else {
                    if (scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                        theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                    }
                    theImage.curX = theImage.parent.width/2 - theImage.width/2
                }
            } else if (theImage.parent.mouseDragType === 2) {

                if (theImage.parent.imageLeftUpX < 0 && theImage.parent.imageLeftUpY < 0) {

                    if (scannerVariables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        if (scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                            theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        } else {
                            theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                        }
                    } else {
                        if (scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        }
                    }
                } else if  (theImage.parent.imageLeftUpX < 0 && theImage.parent.imageLeftUpY > 0) {

                    if (scannerVariables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        if (theImage.parent.imageVisibleLeftUpY < scannerVariables.curWidH) {
                            theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        } else {
                            theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpX
                        }
                    } else {
                        if (theImage.parent.imageVisibleLeftUpY < scannerVariables.curWidH) {
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        }
                    }

                } else if  (theImage.parent.imageLeftUpX > 0 && theImage.parent.imageLeftUpY < 0) {

                    if (theImage.parent.imageVisibleLeftUpX < scannerVariables.curWidW) {
                        if ( scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                            theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        } else {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                        }
                    } else {
                        if ( scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        }
                    }

                } else if (theImage.parent.imageLeftUpX > 0 && theImage.parent.imageLeftUpY > 0) {
                    if (theImage.parent.imageVisibleLeftUpX < scannerVariables.curWidW) {
                        if ( theImage.parent.imageVisibleLeftUpY < scannerVariables.curWidH) {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        } else {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                        }
                    } else {
                        if ( theImage.parent.imageVisibleLeftUpY < scannerVariables.curWidH) {
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
                    if (scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                    }
                    theImage.curY = theImage.parent.height/2 - theImage.height/2
                }
            } else if (theImage.parent.mouseDragType === 1) {
                if (theImage.parent.imageLeftUpY > 0) {
                    theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                } else {
                    if (scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                        theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                    }
                    theImage.curX = theImage.parent.width/2 - theImage.width/2

                }
            } else if (theImage.parent.mouseDragType === 2) {

                if (theImage.parent.imageLeftUpX < 0 && theImage.parent.imageLeftUpY < 0 && (scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0 || mattingvariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0)) {

                    if (scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        if (scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                            theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        } else {
                            theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                        }
                    } else {
                        if (scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        }
                    }

                } else if  (theImage.parent.imageLeftUpX < 0 && theImage.parent.imageLeftUpY > 0) {

                    if (scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX > 0) {
                        if (theImage.parent.imageVisibleLeftUpY < scannerVariables.curWidH) {
                            theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        } else {
                            theImage.curX = theImage.curX + scannerVariables.curWidW - theImage.width * theImage.scale + theImage.parent.imageVisibleLeftUpX
                        }
                    } else {
                        if (theImage.parent.imageVisibleLeftUpY < scannerVariables.curWidH) {
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        }
                    }

                } else if  (theImage.parent.imageLeftUpX > 0 && theImage.parent.imageLeftUpY < 0) {
                    if (theImage.parent.imageVisibleLeftUpX < scannerVariables.curWidW) {
                        if ( scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                            theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        } else {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                        }
                    } else {
                        if ( scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY > 0) {
                            theImage.curY = theImage.curY + scannerVariables.curWidH - theImage.height * theImage.scale + theImage.parent.imageVisibleLeftUpY
                        }
                    }

                } else if (theImage.parent.imageLeftUpX > 0 && theImage.parent.imageLeftUpY > 0) {

                    if (theImage.parent.imageVisibleLeftUpX < scannerVariables.curWidW) {
                        if ( theImage.parent.imageVisibleLeftUpY < scannerVariables.curWidH) {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        } else {
                            theImage.curX = theImage.curX - theImage.parent.imageLeftUpX
                        }
                    } else {
                        if ( theImage.parent.imageVisibleLeftUpY < scannerVariables.curWidH) {
                            theImage.curY = theImage.curY - theImage.parent.imageLeftUpY
                        }
                    }
                }
            } else {
                resetImagePostion(theImage)
            }
        }
    }
}

