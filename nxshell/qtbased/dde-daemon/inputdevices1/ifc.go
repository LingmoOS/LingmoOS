// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"github.com/godbus/dbus/v5"
	langselector "github.com/linuxdeepin/dde-daemon/langselector1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

func (m *Mouse) Reset() *dbus.Error {
	for _, key := range m.setting.ListKeys() {
		m.setting.Reset(key)
	}
	return nil
}

func (m *Mouse) Enable(enabled bool) *dbus.Error {
	if err := m.enable(enabled); err != nil {
		return dbusutil.ToError(err)
	}

	return nil
}

func (tp *TrackPoint) Reset() *dbus.Error {
	for _, key := range tp.setting.ListKeys() {
		tp.setting.Reset(key)
	}
	return nil
}

func (tpad *Touchpad) Reset() *dbus.Error {
	for _, key := range tpad.setting.ListKeys() {
		tpad.setting.Reset(key)
	}
	return nil
}

func (tpad *Touchpad) Enable(enabled bool) *dbus.Error {
	tpad.enable(enabled)
	return nil
}

func (w *Wacom) Reset() *dbus.Error {
	for _, key := range w.setting.ListKeys() {
		w.setting.Reset(key)
	}
	for _, key := range w.stylusSetting.ListKeys() {
		w.stylusSetting.Reset(key)
	}
	for _, key := range w.eraserSetting.ListKeys() {
		w.eraserSetting.Reset(key)
	}
	return nil
}

func (kbd *Keyboard) Reset() *dbus.Error {
	for _, key := range kbd.setting.ListKeys() {
		kbd.setting.Reset(key)
	}
	return nil
}

func (kbd *Keyboard) LayoutList() (map[string]string, *dbus.Error) {
	locales := langselector.GetLocales()
	result := kbd.layoutMap.filterByLocales(locales)

	kbd.PropsMu.RLock()
	for _, layout := range kbd.UserLayoutList {
		layoutDetail := kbd.layoutMap[layout]
		result[layout] = layoutDetail.Description
	}
	kbd.PropsMu.RUnlock()

	return result, nil
}

func (kbd *Keyboard) GetLayoutDesc(layout string) (string, *dbus.Error) {
	if len(layout) == 0 {
		return "", nil
	}

	value, ok := kbd.layoutMap[layout]
	if !ok {
		return "", nil
	}

	return value.Description, nil
}

func (kbd *Keyboard) AddUserLayout(layout string) *dbus.Error {
	err := kbd.checkLayout(layout)
	if err != nil {
		return dbusutil.ToError(errInvalidLayout)
	}

	kbd.addUserLayout(layout)
	return nil
}

func (kbd *Keyboard) DeleteUserLayout(layout string) *dbus.Error {
	kbd.delUserLayout(layout)
	return nil
}

func (kbd *Keyboard) AddLayoutOption(option string) *dbus.Error {
	kbd.addUserOption(option)
	return nil
}

func (kbd *Keyboard) DeleteLayoutOption(option string) *dbus.Error {
	kbd.delUserOption(option)
	return nil
}

func (kbd *Keyboard) ClearLayoutOption() *dbus.Error {
	kbd.UserOptionList.Set([]string{})
	return nil
}

func (kbd *Keyboard) ToggleNextLayout() *dbus.Error {
	kbd.toggleNextLayout()
	return nil
}
