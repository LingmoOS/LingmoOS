// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"syscall"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
)

func (m *Grub2) enableShutdown() {
	if m.inhibitFd != -1 {
		err := syscall.Close(int(m.inhibitFd))
		if err != nil {
			logger.Infof("enable shutdown: fd:%d, err:%s\n", m.inhibitFd, err)
		} else {
			logger.Infof("enable shutdown")
		}
		m.inhibitFd = -1
		m.dbusObj.RemoveAllHandlers()
	}
}

func (m *Grub2) preventShutdown() {
	if m.inhibitFd == -1 {
		fd, err := inhibit("shutdown", dbusServiceName,
			"Updating the system, please shut down or reboot later.")
		logger.Infof("prevent shutdown: fd:%v\n", fd)
		if err != nil {
			logger.Infof("prevent shutdown failed: fd:%v, err:%v\n", fd, err)
			return
		}
		m.inhibitFd = fd
	}
	// handle login1 restart
	_, _ = m.dbusObj.ConnectNameOwnerChanged(func(name string, oldOwner string, newOwner string) {
		if name == "org.freedesktop.login1" && newOwner != "" && oldOwner == "" {
			if m.inhibitFd != -1 { // 如果之前存在inhibit时，login1重启需要重新inhibit
				err := syscall.Close(int(m.inhibitFd))
				m.inhibitFd = -1
				if err != nil {
					logger.Warning("failed to close fd:", err)
					return
				}
				fd, err := inhibit("shutdown", dbusServiceName,
					"Updating the system, please shut down or reboot later.")
				logger.Infof("prevent shutdown: fd:%v\n", fd)
				if err != nil {
					logger.Warning(err)
				} else {
					m.inhibitFd = fd
				}
			}
		}
	})
	// end handle login1 restart
}

func inhibit(what, who, why string) (dbus.UnixFD, error) {
	systemConn, err := dbus.SystemBus()
	if err != nil {
		return 0, err
	}
	m := login1.NewManager(systemConn)
	return m.Inhibit(0, what, who, why, "block")
}
