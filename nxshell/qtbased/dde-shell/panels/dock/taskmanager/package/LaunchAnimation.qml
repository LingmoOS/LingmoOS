// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.99

import org.deepin.ds 1.0
import org.deepin.ds.dock 1.0

SequentialAnimation {
    enum Direction {
        Down,
        Left,
        Up,
        Right
    }
    property Item target: parent
    required property int direction
    required property int launchSpace
    id: root
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0)
        to: value(0.8)
        duration: 40
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.8)
        to: value(1)
        duration: 40
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(1)
        to: value(0.6)
        duration: 40
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.6)
        to: value(0.6)
        duration: 40
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.6)
        to: value(0.9)
        duration: 40
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.9)
        to: value(0.8)
        duration: 40
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.8)
        to: value(0.6)
        duration: 40
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.6)
        to: value(0.8)
        duration: 80
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.8)
        to: value(0.7)
        duration: 40
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.7)
        to: value(0.7)
        duration: 320
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(0.7)
        to: value(-0.2)
        duration: 240
    }
    PropertyAnimation {
        target: root.target
        property: getAxis()
        from: value(-0.2)
        to: value(0)
        duration: 80
    }


    onStarted: {
        target.anchors.centerIn = null
    }
    onStopped: {
        target.anchors.centerIn = target.parent
    }

    function getAxis() {
        if (Panel.position === Dock.Top || Panel.position === Dock.Bottom)
            return "y"
        else
            return "x"
    }

    function value(amplitude) {
        switch (root.direction) {
        case LaunchAnimation.Direction.Down:
            return target.y + root.launchSpace * amplitude
        case LaunchAnimation.Direction.Up:
            return target.y - root.launchSpace * amplitude
        case LaunchAnimation.Direction.Right:
            return target.x + root.launchSpace * amplitude
        case LaunchAnimation.Direction.Left:
            return target.x - root.launchSpace * amplitude
        }
    }
}
