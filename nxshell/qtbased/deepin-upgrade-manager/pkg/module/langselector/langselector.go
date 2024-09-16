// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package langselector

import "github.com/godbus/dbus/v5"

const (
	dbusDest      = "com.deepin.daemon.LangSelector"
	dbusPath      = "/com/deepin/daemon/LangSelector"
	dbusInterface = dbusDest
)

func GetCurrentLocale() (string, error) {
	var locale string
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		return locale, err
	}

	grubServiceObj := sessionBus.Object(dbusDest,
		dbusPath)

	var ret dbus.Variant
	err = grubServiceObj.Call("org.freedesktop.DBus.Properties.Get", 0, dbusInterface, "CurrentLocale").Store(&ret)
	if err != nil {
		return locale, err
	}
	v := ret.Value().(string)
	return v, nil
}
