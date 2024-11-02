/*
 * Copyright 2018 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright 2017 The Qt Company Ltd.
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 */


import QtQuick 2.6
import org.kde.qqc2desktopstyle.private 1.0 as StylePrivate

StylePrivate.StyleItem {
    elementType: "focusrect"
    // those random numbers come from QQC1 desktop style
    anchors {
        top: parent.top
        bottom: parent.bottom
        topMargin: parent.topPadding - 1
        bottomMargin: parent.bottomPadding - 1
    }
    // this is explicitly not using left anchor for auto mirroring
    // since the label's leftPadding/rightPadding already accounts for that
    x: parent.leftPadding - 2
    width: parent.implicitWidth - parent.leftPadding - parent.rightPadding + 3
    visible: control.activeFocus
}
