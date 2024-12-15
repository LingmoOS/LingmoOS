Item {

//! [0]
Rectangle {
    id: backgroundRect

    color: Qt.rgba(1, 0, 0, 0.4)
    width: 200
    height: 200
    radius: 8
}

BoxInsetShadow {
    anchors.fill: backgroundRect
    z: DTK.AboveOrder
    shadowBlur: 2
    spread: 1
    shadowColor: Qt.rgba(0, 0, 0)
    cornerRadius: backgroundRect.radius
}
//! [0]
//! [1]
Rectangle {
    id: backgroundRect

    color: Qt.rgba(1, 0, 0, 0.4)
    width: 200
    height: 200
    radius: 8
}

BoxInsetShadow {
    anchors.fill: backgroundRect
    z: DTK.AboveOrder
    shadowBlur: 20
    spread: 1
    shadowColor: Qt.rgba(0, 0, 0)
    cornerRadius: backgroundRect.radius
}
//! [1]
//! [2]
Rectangle {
    id: backgroundRect

    color: Qt.rgba(1, 0, 0, 0.4)
    width: 200
    height: 200
    radius: 8
}

BoxInsetShadow {
    anchors.fill: backgroundRect
    z: DTK.AboveOrder
    shadowBlur: 40
    spread: 1
    shadowColor: Qt.rgba(0, 0, 0)
    cornerRadius: backgroundRect.radius
}
//! [2]
//! [3]
Rectangle {
    id: backgroundRect

    color: Qt.rgba(1, 0, 0, 0.4)
    width: 200
    height: 200
    radius: 8
}

BoxInsetShadow {
    anchors.fill: backgroundRect
    z: DTK.AboveOrder
    shadowBlur: 2
    spread: 10
    shadowColor: Qt.rgba(0, 0, 0)
    cornerRadius: backgroundRect.radius
}
//! [3]
//! [4]
Rectangle {
    id: backgroundRect

    color: Qt.rgba(1, 0, 0, 0.4)
    width: 200
    height: 200
    radius: 8
}

BoxInsetShadow {
    anchors.fill: backgroundRect
    z: DTK.AboveOrder
    shadowBlur: 2
    spread: 20
    shadowColor: Qt.rgba(0, 0, 0)
    cornerRadius: backgroundRect.radius
}
//! [4]

}