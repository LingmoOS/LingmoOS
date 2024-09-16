// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package dbus

import (
	"github.com/godbus/dbus/v5"
	accounts "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.accounts1"
)

func NewAccounts(systemConn *dbus.Conn) accounts.Accounts {
	return accounts.NewAccounts(systemConn)
}

func NewUserByName(systemConn *dbus.Conn, name string) (accounts.User, error) {
	m := NewAccounts(systemConn)
	userPath, err := m.FindUserByName(0, name)
	if err != nil {
		return nil, err
	}
	return accounts.NewUser(systemConn, dbus.ObjectPath(userPath))
}

func NewUserByUid(systemConn *dbus.Conn, uid string) (accounts.User, error) {
	m := NewAccounts(systemConn)
	userPath, err := m.FindUserById(0, uid)
	if err != nil {
		return nil, err
	}
	return accounts.NewUser(systemConn, dbus.ObjectPath(userPath))
}
