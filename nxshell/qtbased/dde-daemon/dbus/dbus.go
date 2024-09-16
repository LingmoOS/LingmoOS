// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package dbus

import (
	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib"
)

// IsSessionBusActivated check the special session bus name whether activated
func IsSessionBusActivated(dest string) bool {
	if !lib.UniqueOnSession(dest) {
		return true
	}

	bus, _ := dbus.SessionBus()
	releaseDBusName(bus, dest)
	return false
}

// IsSystemBusActivated check the special system bus name whether activated
func IsSystemBusActivated(dest string) bool {
	if !lib.UniqueOnSystem(dest) {
		return true
	}

	bus, _ := dbus.SystemBus()
	releaseDBusName(bus, dest)
	return false
}

func releaseDBusName(bus *dbus.Conn, name string) {
	if bus != nil {
		_, _ = bus.ReleaseName(name)
	}
}
