// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	"path/filepath"
	"syscall"
)

func (u *User) getCreatedTime() (uint64, error) {
	createdTime, err := getCreatedTimeFromBashLogout(u.HomeDir)
	if err == nil {
		return createdTime, nil
	}
	return getCreatedTimeFromUserConfig(u.UserName)
}

// '.bash_logout' from '/etc/skel/.bash_logout'
func getCreatedTimeFromBashLogout(home string) (uint64, error) {
	return getCreatedTimeFromFile(filepath.Join(home, ".bash_logout"))
}

// using deepin accounts dbus created user, will created this configuration file
func getCreatedTimeFromUserConfig(username string) (uint64, error) {
	return getCreatedTimeFromFile(filepath.Join("/var/lib/AccountsService/deepin/users",
		username))
}

func getCreatedTimeFromFile(filename string) (uint64, error) {
	var stat syscall.Stat_t
	err := syscall.Stat(filename, &stat)
	if err != nil {
		return 0, err
	}
	// Ctim: recent change time
	return uint64(stat.Ctim.Sec), nil
}
