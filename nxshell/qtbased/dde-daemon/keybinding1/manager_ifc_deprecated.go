// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

// List list all shortcut
func (m *Manager) List() (shortcuts string, busErr *dbus.Error) {
	return m.ListAllShortcuts()
}

// Add add custom shortcut
//
// name: the name
// action: the command line
// keystroke: the keystroke
// ret0: ""
// ret1: false
// ret2: error
func (m *Manager) Add(name, action, keystroke string) (ret0 string, ret1 bool, busErr *dbus.Error) {
	_, _, err := m.AddCustomShortcut(name, action, keystroke)
	return "", false, err
}

// Delete delete shortcut by id and type
//
// id: the specail id
// ty: the special type
// ret0: error info
func (m *Manager) Delete(id string, type0 int32) *dbus.Error {
	if type0 != shortcuts.ShortcutTypeCustom {
		return dbusutil.ToError(ErrInvalidShortcutType{type0})
	}

	return m.DeleteCustomShortcut(id)
}

// Disable cancel the shortcut
func (m *Manager) Disable(id string, type0 int32) *dbus.Error {
	return m.ClearShortcutKeystrokes(id, type0)
}

// CheckAvaliable 检查快捷键序列是否可用
// 返回值1 是否可用;
// 返回值2 与之冲突的快捷键的详细信息，是JSON字符串。如果没有冲突，则为空字符串。
func (m *Manager) CheckAvaliable(keystroke string) (available bool, shortcut string, busErr *dbus.Error) {
	detail, err := m.LookupConflictingShortcut(keystroke)
	if err != nil {
		return false, "", err
	}
	return detail == "", detail, nil
}

// ModifiedAccel modify shortcut keystroke
//
// id: the special id
// ty: the special type
// keystroke: new keystroke
// add: if true, add keystroke for the special id; else delete it
// ret0: always equal false
// ret1: always equal ""
// ret2: error
func (m *Manager) ModifiedAccel(id string, type0 int32, keystroke string, add bool) (ret0 bool, ret1 string,
	busErr *dbus.Error) {
	if add {
		return false, "", m.AddShortcutKeystroke(id, type0, keystroke)
	} else {
		return false, "", m.DeleteShortcutKeystroke(id, type0, keystroke)
	}
}

// Query query shortcut detail info by id and type
func (m *Manager) Query(id string, type0 int32) (shortcut string, busErr *dbus.Error) {
	return m.GetShortcut(id, type0)
}

// GrabScreen grab screen for getting the key pressed
func (m *Manager) GrabScreen() *dbus.Error {
	return m.SelectKeystroke()
}
