// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Shapes 1.11
import org.deepin.dtk 1.0 as D

Rectangle {
    width: 200; height: 200
    color: "red"

    Shape {
        ShapePath {
            fillColor: "blue"
            strokeColor: "yellow"
            strokeStyle: ShapePath.SolidLine
            strokeWidth: 1
            pathElements: path
        }
    }

    D.ArrowBoxPath {
        id: path
        arrowX: 50
        arrowY: 10
        arrowWidth: 20
        arrowHeight: 20
        arrowDirection: D.ArrowBoxPath.Up
        width: 100
        height: 100
        roundedRadius: 8
        spread: 4
    }
}
