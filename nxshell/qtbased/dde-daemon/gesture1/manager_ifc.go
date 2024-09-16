// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package gesture1

import (
	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

func (m *Manager) SetLongPressDuration(duration uint32) *dbus.Error {
	if m.tsSetting.GetInt(tsSchemaKeyLongPress) == int32(duration) {
		return nil
	}
	err := m.sysDaemon.SetLongPressDuration(0, duration)
	if err != nil {
		return dbusutil.ToError(err)
	}
	m.tsSetting.SetInt(tsSchemaKeyLongPress, int32(duration))
	return nil
}

func (m *Manager) GetLongPressDuration() (duration uint32, busErr *dbus.Error) {
	return uint32(m.tsSetting.GetInt(tsSchemaKeyLongPress)), nil
}

func (m *Manager) SetShortPressDuration(duration uint32) *dbus.Error {
	if m.tsSetting.GetInt(tsSchemaKeyShortPress) == int32(duration) {
		return nil
	}
	err := m.gesture.SetShortPressDuration(0, duration)
	if err != nil {
		return dbusutil.ToError(err)
	}
	m.tsSetting.SetInt(tsSchemaKeyShortPress, int32(duration))
	return nil
}

func (m *Manager) GetShortPressDuration() (duration uint32, busErr *dbus.Error) {
	return uint32(m.tsSetting.GetInt(tsSchemaKeyShortPress)), nil
}

func (m *Manager) SetEdgeMoveStopDuration(duration uint32) *dbus.Error {
	if m.tsSetting.GetInt(tsSchemaKeyShortPress) == int32(duration) {
		return nil
	}
	err := m.gesture.SetEdgeMoveStopDuration(0, duration)
	if err != nil {
		return dbusutil.ToError(err)
	}
	m.tsSetting.SetInt(tsSchemaKeyEdgeMoveStop, int32(duration))
	return nil
}

func (m *Manager) GetEdgeMoveStopDuration() (duration uint32, busErr *dbus.Error) {
	return uint32(m.tsSetting.GetInt(tsSchemaKeyEdgeMoveStop)), nil
}
