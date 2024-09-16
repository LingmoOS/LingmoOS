// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Window {
    id: control

    maximumWidth: Screen.desktopAvailableWidth
    maximumHeight: Screen.desktopAvailableHeight
    minimumWidth: DS.Style.dialogWindow.width
    minimumHeight: DS.Style.dialogWindow.height
    D.DWindow.enabled: true
    D.DWindow.enableSystemResize: false
    D.DWindow.motifFunctions: D.WindowManagerHelper.FUNC_ALL & ~D.WindowManagerHelper.FUNC_RESIZE
    D.DWindow.wmWindowTypes: D.WindowManagerHelper.DialogType
    D.DWindow.enableBlurWindow: true
    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.MSWindowsFixedSizeDialogHint
    D.ColorSelector.family: D.Palette.CrystalColor
    color: active ? D.DTK.palette.window : D.DTK.inactivePalette.window
    height: content.height
    width: content.width

    property alias header: titleBar.sourceComponent
    property string icon
    default property alias content: contentLoader.children
    property alias palette : content.palette

    Control {
        id: content
        palette: control.active ? D.DTK.palette : D.DTK.inactivePalette
        contentItem: ColumnLayout {
            id: layout
            spacing: 0

            Loader {
                id: titleBar
                z: D.DTK.TopOrder
                sourceComponent: DialogTitleBar {
                    enableInWindowBlendBlur: true
                    icon.name: control.icon
                }
            }

            Item {
                id: contentLoader
                Layout.fillWidth: true
                Layout.preferredHeight: childrenRect.height
                Layout.leftMargin: DS.Style.dialogWindow.contentHMargin
                Layout.rightMargin: DS.Style.dialogWindow.contentHMargin
            }
        }
    }

    onClosing: {
        // close can't reset sub control's hovered state. pms Bug:168405
        // if we need to close, we can add closing handler to set `close.acceped = true`
        close.accepted = false
        hide()
    }
}
