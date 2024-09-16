// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"fmt"
	"sync"

	"github.com/linuxdeepin/dde-daemon/loader"

	_ "github.com/linuxdeepin/dde-daemon/audio1"
	_ "github.com/linuxdeepin/dde-daemon/bluetooth1"
	_ "github.com/linuxdeepin/dde-daemon/screenedge1"

	// depends: network
	_ "github.com/linuxdeepin/dde-daemon/calltrace"
	_ "github.com/linuxdeepin/dde-daemon/clipboard1"
	_ "github.com/linuxdeepin/dde-daemon/debug"

	_ "github.com/linuxdeepin/dde-daemon/gesture1"
	_ "github.com/linuxdeepin/dde-daemon/housekeeping"
	_ "github.com/linuxdeepin/dde-daemon/inputdevices1"
	_ "github.com/linuxdeepin/dde-daemon/keybinding1"
	_ "github.com/linuxdeepin/dde-daemon/lastore1"

	_ "github.com/linuxdeepin/dde-daemon/network1"
	_ "github.com/linuxdeepin/dde-daemon/screensaver1"
	_ "github.com/linuxdeepin/dde-daemon/service_trigger"
	_ "github.com/linuxdeepin/dde-daemon/session/power1"
	_ "github.com/linuxdeepin/dde-daemon/session/uadpagent1"
	_ "github.com/linuxdeepin/dde-daemon/sessionwatcher1"
	_ "github.com/linuxdeepin/dde-daemon/systeminfo1"
	_ "github.com/linuxdeepin/dde-daemon/timedate1"
	_ "github.com/linuxdeepin/dde-daemon/trayicon1"
	_ "github.com/linuxdeepin/dde-daemon/x_event_monitor1"
)

var (
	moduleLocker sync.Mutex
)

func (s *SessionDaemon) checkDependencies(module loader.Module, enabled bool) error {
	if enabled {
		depends := module.GetDependencies()
		for _, n := range depends {
			if !s.getConfigValue(n) {
				return fmt.Errorf("Dependency lose: %v", n)
			}
		}
		return nil
	}

	for _, m := range loader.List() {
		if m == nil || m.Name() == module.Name() {
			continue
		}

		if m.IsEnable() && isStrInList(module.Name(), m.GetDependencies()) {
			return fmt.Errorf("Can not disable this module '%s', because of it was depended by'%s'",
				module.Name(), m.Name())
		}
	}
	return nil
}

func isStrInList(item string, list []string) bool {
	for _, v := range list {
		if item == v {
			return true
		}
	}
	return false
}
