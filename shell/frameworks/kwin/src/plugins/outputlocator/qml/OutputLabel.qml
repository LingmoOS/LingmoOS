/*
    SPDX-FileCopyrightText: 2012 Dan Vratil <dvratil@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2 as LingmoUI

Rectangle {
    id: root;

    property string outputName;
    property size resolution;
    property double scale;

    color: LingmoUI.Theme.backgroundColor

    implicitWidth: childrenRect.width + 2 * childrenRect.x
    implicitHeight: childrenRect.height + 2 * childrenRect.y

    LingmoComponents3.Label {
        id: displayName
        x: LingmoUI.Units.largeSpacing * 2
        y: LingmoUI.Units.largeSpacing
        font.pointSize: LingmoUI.Theme.defaultFont.pointSize * 3
        text: root.outputName;
        wrapMode: Text.WordWrap;
        horizontalAlignment: Text.AlignHCenter;
    }

    LingmoComponents3.Label {
        id: modeLabel;
        anchors {
            horizontalCenter: displayName.horizontalCenter
            top: displayName.bottom
        }
        text: resolution.width + "x" + resolution.height +
              (root.scale !== 1 ? "@" + Math.round(root.scale * 100.0) + "%": "")
        horizontalAlignment: Text.AlignHCenter;
    }
}
