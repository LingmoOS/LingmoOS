// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package trayicon

import (
	"errors"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	x "github.com/linuxdeepin/go-x11-client"
)

const (
	dbusServiceName = "org.deepin.dde.TrayManager1"
	dbusInterface   = dbusServiceName
	dbusPath        = "/org/deepin/dde/TrayManager1"
)

func (*TrayManager) GetInterfaceName() string {
	return dbusInterface
}

// Manage方法获取系统托盘图标的管理权。
func (m *TrayManager) Manage() (ok bool, busErr *dbus.Error) {
	logger.Debug("call Manage by dbus")

	err := m.sendClientMsgMANAGER()
	if err != nil {
		logger.Warning(err)
		return false, dbusutil.ToError(err)
	}
	return true, nil
}

// GetName返回传入的系统图标的窗口id的窗口名。
func (m *TrayManager) GetName(win uint32) (name string, busErr *dbus.Error) {
	m.mutex.Lock()
	icon, ok := m.icons[x.Window(win)]
	m.mutex.Unlock()
	if !ok {
		return "", dbusutil.ToError(errors.New("icon not found"))
	}
	return icon.getName(), nil
}

// EnableNotification设置对应id的窗口是否可以通知。
func (m *TrayManager) EnableNotification(win uint32, enabled bool) *dbus.Error {
	m.mutex.Lock()
	icon, ok := m.icons[x.Window(win)]
	m.mutex.Unlock()
	if !ok {
		return dbusutil.ToError(errors.New("icon not found"))
	}

	icon.mu.Lock()
	icon.notify = enabled
	icon.mu.Unlock()
	return nil
}
