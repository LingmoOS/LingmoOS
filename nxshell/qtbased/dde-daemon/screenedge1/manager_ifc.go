// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package screenedge

import (
	"errors"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

// Enable desktop edge zone detected
//
// 是否启用桌面边缘热区功能
func (m *Manager) EnableZoneDetected(enabled bool) *dbus.Error {
	has, err := m.service.NameHasOwner(wmDBusServiceName)
	if err != nil {
		return dbusutil.ToError(err)
	}

	if !has {
		return dbusutil.ToError(errors.New("deepin-wm is not running"))
	}

	err = m.wm.EnableZoneDetected(0, enabled)
	return dbusutil.ToError(err)
}

// Set left-top edge action
func (m *Manager) SetTopLeft(value string) *dbus.Error {
	m.settings.SetEdgeAction(TopLeft, value)
	return nil
}

// Get left-top edge action
func (m *Manager) TopLeftAction() (value string, busErr *dbus.Error) {
	return m.settings.GetEdgeAction(TopLeft), nil
}

// Set left-bottom edge action
func (m *Manager) SetBottomLeft(value string) *dbus.Error {
	m.settings.SetEdgeAction(BottomLeft, value)
	return nil
}

// Get left-bottom edge action
func (m *Manager) BottomLeftAction() (value string, busErr *dbus.Error) {
	return m.settings.GetEdgeAction(BottomLeft), nil
}

// Set right-top edge action
func (m *Manager) SetTopRight(value string) *dbus.Error {
	m.settings.SetEdgeAction(TopRight, value)
	return nil
}

// Get right-top edge action
func (m *Manager) TopRightAction() (value string, busErr *dbus.Error) {
	return m.settings.GetEdgeAction(TopRight), nil
}

// Set right-bottom edge action
func (m *Manager) SetBottomRight(value string) *dbus.Error {
	m.settings.SetEdgeAction(BottomRight, value)
	return nil
}

// Get right-bottom edge action
func (m *Manager) BottomRightAction() (value string, busErr *dbus.Error) {
	return m.settings.GetEdgeAction(BottomRight), nil
}
