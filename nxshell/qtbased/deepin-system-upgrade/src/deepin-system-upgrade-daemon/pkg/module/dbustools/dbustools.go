// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package dbustools

import (
	"github.com/godbus/dbus"
	"github.com/linuxdeepin/go-lib/log"
)

var logger = log.NewLogger("deepin-system-upgrade/module/dbustools")

func GetProperty(dest, dbusInterface, property string, path dbus.ObjectPath) (dbus.Variant, error) {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning("failed to init system bus:", err)
		return dbus.MakeVariant(nil), err
	}
	obj := sysBus.Object(dest, path)
	return obj.GetProperty(dbusInterface + "." + property)
}

func DBusMethodCaller(dest, method string, path dbus.ObjectPath, parameter ...interface{}) error {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	obj := sysBus.Object(dest, path)
	logger.Debugf("print method %s with parameter %v:", method, parameter)
	err = obj.Call(method, 0, parameter...).Err
	if err != nil {
		logger.Warning("failed to call dbus interface:", err)
		return err
	}
	return nil
}
