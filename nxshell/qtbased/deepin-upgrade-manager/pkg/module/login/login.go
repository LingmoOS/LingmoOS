// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package login

import (
	"deepin-upgrade-manager/pkg/logger"
	"syscall"

	"github.com/godbus/dbus/v5"
)

const (
	dbusDest      = "org.freedesktop.login1"
	dbusPath      = "/org/freedesktop/login1"
	dbusInterface = "org.freedesktop.login1.Manager"
)

func Inhibit(what, who, why string) (dbus.UnixFD, error) {
	var fd dbus.UnixFD
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return fd, err
	}
	loginServiceObj := sysBus.Object(dbusDest,
		dbusPath)
	metho := dbusInterface + ".Inhibit"

	err = loginServiceObj.Call(metho, 0, what, who, why, "block").Store(&fd)
	if err != nil {
		return fd, err
	}
	return fd, nil
}

func Close(fd dbus.UnixFD) {
	if fd != -1 {
		err := syscall.Close(int(fd))
		if err != nil {
			logger.Infof("enable shutdown: fd:%d, err:%s\n", fd, err)
		} else {
			logger.Infof("enable shutdown")
		}
	}
}
