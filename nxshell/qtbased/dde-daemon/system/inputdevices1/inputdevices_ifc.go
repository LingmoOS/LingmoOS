// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices1

import (
	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

func (m *InputDevices) SetWakeupDevices(sender dbus.Sender, path string, value string) *dbus.Error {
	err := m.setWakeupDevices(path, value)
	return dbusutil.ToError(err)
}
