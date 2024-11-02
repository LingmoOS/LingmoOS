import QtQuick 2.12

Item {
    property point mousePos: Qt.point(-1, -1)
    property point mouseCurPos: Qt.point(-1, -1)
    property double curWidW: 0
    property double curWidH: 0
    property bool isOverWidSize: false
    //缩放比例常数
    property int scalingconstant: 100
    property int imageAniDuration: 300
    property int imageZoomSpeed: 15
    //4个角度
    property int angle90: 90
    property int angle180: 180
    property int angle270: 270
    property int angle360: 360
    property bool wheelChange: false
    property int operateWay: 0
    property int operateImageWay:12
    property string format: ""
}
