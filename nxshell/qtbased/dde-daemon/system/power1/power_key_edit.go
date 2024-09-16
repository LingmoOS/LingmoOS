// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"errors"

	"github.com/godbus/dbus/v5"
	ConfigManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	"github.com/linuxdeepin/go-lib/utils"
)

func interfaceToArrayString(v interface{}) (d []interface{}) {
	if utils.IsInterfaceNil(v) {
		return
	}

	d, ok := v.([]interface{})
	if !ok {
		logger.Errorf("interfaceToArrayString() failed: %#v", v)
		return
	}
	return
}

func (m *Manager) setDsgData(key string, value interface{}, dsg ConfigManager.Manager) error {
	if dsg == nil {
		return errors.New("setDsgData dsg is nil")
	}
	err := dsg.SetValue(0, key, dbus.MakeVariant(value))
	if err != nil {
		logger.Warningf("setDsgData key : %s. err : %v", key, err)
		return err
	}
	logger.Debugf("setDsgData key : %s , value : %v", key, value)
	return nil
}
