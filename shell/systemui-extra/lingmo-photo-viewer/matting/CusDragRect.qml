import QtQuick 2.12

Rectangle {
    id: cusdrag
    color: "transparent"
    width: 20
    height: 20
    readonly property int posLeftTop: Qt.SizeFDiagCursor
    readonly property int posLeft: Qt.SizeHorCursor
    readonly property int posLeftBottom: Qt.SizeBDiagCursor
    readonly property int posTop: Qt.SizeVerCursor
    readonly property int posBottom: Qt.SizeVerCursor
    readonly property int posRightTop: Qt.SizeBDiagCursor
    readonly property int posRight: Qt.SizeHorCursor
    readonly property int posRightBottom: Qt.SizeFDiagCursor

    readonly property string imgLeftTop: "qrc:/res/res/cut/leftup.png"
    readonly property string imgLeft: "qrc:/res/res/cut/vertical.png"
    readonly property string imgLeftBottom: "qrc:/res/res/cut/leftdown.png"
    readonly property string imgTop: "qrc:/res/res/cut/horizontal.png"
    readonly property string imgBottom: "qrc:/res/res/cut/horizontal.png"
    readonly property string imgRightTop: "qrc:/res/res/cut/rightup.png"
    readonly property string imgRight: "qrc:/res/res/cut/vertical.png"
    readonly property string imgRightBottom: "qrc:/res/res/cut/rightdown.png"

    property int posType: Qt.ArrowCursor
    property string imgType: ""
    property int horizontalAlignType: Image.AlignHCenter
    property int verticalAlignType: Image.AlignVCenter
    property var callBackFunc

    signal sigPosChanged(var mousePoint)

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: imgType
        horizontalAlignment: horizontalAlignType
        verticalAlignment: verticalAlignType
    }

    MouseArea{
        id: cusdragmouse
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
        onContainsMouseChanged: {
            if (containsMouse && parent.parent.parent.canOperate) {
                cursorShape = posType;
            } else {
                cursorShape = Qt.ArrowCursor;
            }
        }
        onPositionChanged: {
            if (pressed && parent.parent.parent.canOperate) {
                var pos = cusdragmouse.mapToItem(mattingBgImage, mouse.x, mouse.y)
                mattingbasefunction.imageActuralLeftUpPos(mattingImage,mattingImage.curX,mattingImage.curY)
                if(Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle90 ||
                   Math.abs(mattingImage.rotateTo % mattingvariables.angle360) == mattingvariables.angle270)
                {
                    if ((pos.x > imageLeftUpX) && (pos.y > imageLeftUpY) &&
                            (pos.x < imageLeftUpX + mattingImage.height*mattingImage.curScale) &&
                            (pos.y < imageLeftUpY + mattingImage.width*mattingImage.curScale))
                    {
                        sigPosChanged(pos)
                        updateGrid(true);
                        if (parent.parent.parent.fixedScale === 0)
                        {
                            cutFreeSize();
                        }
                    }
                }
                else
                {
                    if ((pos.x > imageLeftUpX) && (pos.y > imageLeftUpY) &&
                            (pos.x < imageLeftUpX + mattingImage.width*mattingImage.curScale) &&
                            (pos.y < imageLeftUpY + mattingImage.height*mattingImage.curScale))
                    {
                        sigPosChanged(pos)
                        updateGrid(true);
                        if (parent.parent.parent.fixedScale === 0)
                        {
                            cutFreeSize();
                        }
                    }
                }
            }
        }
        onReleased: {
            if (parent.parent.parent.canOperate)
            {
                callBackFunc();
                updateGrid(false);
            }
        }
    }
}
