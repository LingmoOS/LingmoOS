// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package trayicon

import (
	x "github.com/linuxdeepin/go-x11-client"
)

func isValidWindow(win x.Window) bool {
	reply, err := x.GetWindowAttributes(XConn, win).Reply(XConn)
	return reply != nil && err == nil
}

func findRGBAVisualID() x.VisualID {
	screen := XConn.GetDefaultScreen()
	for _, dinfo := range screen.AllowedDepths {
		if dinfo.Depth == 32 {
			for _, vinfo := range dinfo.Visuals {
				return vinfo.Id
			}
		}
	}
	return screen.RootVisual
}
