// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package trayicon

import (
	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/ext/composite"
	"github.com/linuxdeepin/go-x11-client/ext/damage"
	"github.com/linuxdeepin/go-x11-client/util/atom"

	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

func init() {
	loader.Register(NewDaemon(logger))
}

var (
	logger = log.NewLogger("daemon/trayicon")

	XConn *x.Conn

	XA_NET_SYSTEM_TRAY_S0         x.Atom
	XA_NET_SYSTEM_TRAY_OPCODE     x.Atom
	XA_NET_SYSTEM_TRAY_VISUAL     x.Atom
	XA_NET_SYSTEM_TRAY_ORIENTAION x.Atom
	XA_MANAGER                    x.Atom
)

func initX() {
	_, err := damage.QueryVersion(XConn, damage.MajorVersion, damage.MinorVersion).Reply(XConn)
	if err != nil {
		logger.Warning(err)
	}

	_, err = composite.QueryVersion(XConn, composite.MajorVersion, composite.MinorVersion).Reply(XConn)
	if err != nil {
		logger.Warning(err)
	}

	XA_NET_SYSTEM_TRAY_S0, _ = atom.GetVal(XConn, "_NET_SYSTEM_TRAY_S0")
	XA_NET_SYSTEM_TRAY_OPCODE, _ = atom.GetVal(XConn, "_NET_SYSTEM_TRAY_OPCODE")
	XA_NET_SYSTEM_TRAY_VISUAL, _ = atom.GetVal(XConn, "_NET_SYSTEM_TRAY_VISUAL")
	XA_NET_SYSTEM_TRAY_ORIENTAION, _ = atom.GetVal(XConn, "NET_SYSTEM_TRAY_ORIENTAION")
	XA_MANAGER, _ = atom.GetVal(XConn, "MANAGER")
}
