// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package trayicon

import (
	"fmt"

	"sync"

	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/ext/composite"
	"github.com/linuxdeepin/go-x11-client/ext/damage"
	"github.com/linuxdeepin/go-x11-client/util/wm/ewmh"
	"github.com/linuxdeepin/go-x11-client/util/wm/icccm"
)

type TrayIcon struct {
	win    x.Window
	notify bool
	data   []byte // window pixmap data
	damage damage.Damage
	mu     sync.Mutex
}

func NewTrayIcon(win x.Window) *TrayIcon {
	return &TrayIcon{
		win:    win,
		notify: true,
	}
}

func (icon *TrayIcon) getName() string {
	wmName, _ := ewmh.GetWMName(XConn, icon.win).Reply(XConn)
	if wmName != "" {
		return wmName
	}

	wmNameTextProp, err := icccm.GetWMName(XConn, icon.win).Reply(XConn)
	if err == nil {
		wmName, _ := wmNameTextProp.GetStr()
		if wmName != "" {
			return wmName
		}
	}

	wmClass, err := icccm.GetWMClass(XConn, icon.win).Reply(XConn)
	if err == nil {
		return fmt.Sprintf("[%s|%s]", wmClass.Class, wmClass.Instance)
	}

	return ""
}

func (icon *TrayIcon) getPixmapData() ([]byte, error) {
	pixmapId, err := XConn.AllocID()
	if err != nil {
		return nil, err
	}
	defer func() {
		err := XConn.FreeID(pixmapId)
		if err != nil {
			logger.Warning(err)
		}
	}()

	pixmap := x.Pixmap(pixmapId)
	err = composite.NameWindowPixmapChecked(XConn, icon.win, pixmap).Check(XConn)
	if err != nil {
		return nil, err
	}
	defer x.FreePixmap(XConn, pixmap)

	geo, err := x.GetGeometry(XConn, x.Drawable(icon.win)).Reply(XConn)
	if err != nil {
		return nil, err
	}

	img, err := x.GetImage(XConn, x.ImageFormatZPixmap, x.Drawable(pixmap),
		0, 0, geo.Width, geo.Height, (1<<32)-1).Reply(XConn)
	if err != nil {
		return nil, err
	}
	return img.Data, nil
}
