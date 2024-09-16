// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"fmt"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/dde-daemon/system/gesture1"
)

func (*Daemon) SetLongPressDuration(duration uint32) *dbus.Error {
	epath := dbusPath + ".SetLongPressDuration"
	if duration < 1 {
		return dbus.NewError(epath,
			[]interface{}{fmt.Errorf("invalid duration: %d", duration)})
	}
	var m = loader.GetModule("gesture")
	if m == nil {
		return dbus.NewError(epath,
			[]interface{}{"Not found module 'gesture'"})
	}
	m.(*gesture1.Daemon).SetLongPressDuration(int(duration))
	return nil
}
