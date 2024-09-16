// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton

import QtQuick 2.15

import org.deepin.ds.dock 1.0
import org.deepin.dtk as D

Item {
    // clipboardItem and searchItem toolbutton background color
    property D.Palette toolButtonColor: D.Palette {
        normal {
            common: Qt.rgba(1, 1, 1, 0.4)
        }
        normalDark{
            common: Qt.rgba(1, 1, 1, 0.1)
        }
    }

    // clipboardItem and searchItem toolbutton background border color
    property D.Palette toolButtonBorderColor: D.Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 0.1)
        }
        normalDark{
            common: Qt.rgba(1, 1, 1, 0.2)
        }
    }

    // workspace rectangle filled color
    property D.Palette workspaceRectangleColor: D.Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 0.1)
        }
        normalDark {
            common: Qt.rgba(1, 1, 1, 0.1)
        }

        hovered {
            common: Qt.rgba(0, 0, 0, 0.2)
        }
        hoveredDark {
            common: Qt.rgba(0, 0, 0, 0.3)
        }
    }

    // workspace current workspace border color
    property D.Palette workspaceSelectedBorderColor: D.Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 1)
        }
        normalDark{
            common: Qt.rgba(1, 1, 1, 1)
        }
    }

    // workspace not current workspace border color
    property D.Palette workspaceUnselectedBorderColor: D.Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 0.5)
        }
        normalDark {
            common: Qt.rgba(1, 1, 1, 0.5)
        }
        hovered {
            common: Qt.rgba(0, 0, 0, 0.6)
        }
        hoveredDark{
            common: Qt.rgba(1, 1, 1, 0.6)
        }
    }

    // taskmanager
    property D.Palette taskmanagerStatusIndicatorColor: D.Palette {
        normal {
            common: Qt.rgba(0, 0, 0)
        }
        normalDark {
            common: Qt.rgba(1, 1, 1)
        }
    }

    property D.Palette taskmanagerStatusIndicatorBorderColor: D.Palette {
        normal {
            common: Qt.rgba(1, 1, 1, 0.2)
        }
        normalDark {
            common: Qt.rgba(0, 0, 0, 0.2)
        }
    }

    property D.Palette taskmanagerStatusIndicatorActiveColor: D.Palette {
        normal {
            common: Qt.rgba(1, 1, 1, 0.2)
        }
        normalDark {
            common: Qt.rgba(0, 0, 0, 0.35)
        }
    }

    property D.Palette taskmanagerWindowIndicatorColor: D.Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 0.3)
        }
        normalDark {
            common:  Qt.rgba(1, 1, 1, 0.3)
        }
    }

    property D.Palette taskmanagerWindowIndicatorBoderColor: D.Palette {
        normal {
            common: Qt.rgba(1, 1, 1, 0.2)
        }
        normalDark {
            common: Qt.rgba(0, 0, 0, 0.35)
        }
    }

    property D.Palette taskmanagerWindowIndicatorActiveColor: D.Palette {
        normal {
            common: D.DTK.makeColor(D.Color.Highlight)
        }
    }

    property D.Palette showDesktopLineColor: D.Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 0.2)
        }
        normalDark {
            common: Qt.rgba(1, 1, 1, 0.2)
        }
    }

    property D.Palette backgroundPalette: D.Palette {
        normal {
            common: ("transparent")
        }
        hovered {
            crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
        }
    }

    property D.Palette iconTextPalette: D.Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 1)
        }
        normalDark {
            common: Qt.rgba(1, 1, 1, 1)
        }
    }
}
