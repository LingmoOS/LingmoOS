// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk 1.0

Loader {
    enum Direction {
        UpButton = 0,
        DownButton = 1
    }

    property Item view
    property int direction
    active: view.interactive

    sourceComponent: Button {
        flat: true
        enabled: direction === ArrowListViewButton.UpButton ? !view.atYBeginning : !itemsView.atYEnd
        width: DS.Style.arrowListView.stepButtonSize.width
        height: DS.Style.arrowListView.stepButtonSize.height
        icon.name: direction === ArrowListViewButton.UpButton ? DS.Style.arrowListView.upButtonIconName
                                                              : DS.Style.arrowListView.downButtonIconName
        icon.width: DS.Style.arrowListView.stepButtonIconSize.width
        icon.height: DS.Style.arrowListView.stepButtonIconSize.height
        onClicked: direction === ArrowListViewButton.UpButton ? view.decrementCurrentIndex()
                                                              : view.incrementCurrentIndex()
    }
}
