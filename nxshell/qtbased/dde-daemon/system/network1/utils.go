// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network1

// #cgo pkg-config: libudev
// #cgo CFLAGS: -W -Wall -fstack-protector-all -fPIC
// #include <stdlib.h>
// #include "utils_udev.h"
import "C"
import (
	"os/exec"

	"github.com/godbus/dbus/v5"
	networkmanager "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.networkmanager"
)

func getSettingConnectionTimestamp(settings map[string]map[string]dbus.Variant) uint64 {
	a := settings["connection"]["timestamp"].Value()
	if a == nil {
		return 0
	}
	val, ok := a.(uint64)
	if !ok {
		logger.Warning("type of setting connection.timestamp is not uint64")
		return 0
	}
	return val
}

func getSettingConnectionAutoconnect(settings map[string]map[string]dbus.Variant) bool {
	a := settings["connection"]["autoconnect"].Value()
	if a == nil {
		return true
	}
	val, ok := a.(bool)
	if !ok {
		logger.Warning("type of setting connection.autoconnect is not bool")
		return false
	}
	return val
}

func getSettingConnectionType(settings map[string]map[string]dbus.Variant) string {
	return getSettingString(settings, "connection", "type")
}

func getSettingConnectionUuid(settings map[string]map[string]dbus.Variant) string {
	return getSettingString(settings, "connection", "uuid")
}
func getSettingConnectionId(settings map[string]map[string]dbus.Variant) string {
	return getSettingString(settings, "connection", "id")
}

func getSettingString(settings map[string]map[string]dbus.Variant, key1, key2 string) string {
	a := settings[key1][key2].Value()
	if a == nil {
		return ""
	}
	val, ok := a.(string)
	if !ok {
		logger.Warning("type of setting connection.autoconnect is not string")
		return ""
	}
	return val
}

func setDeviceAutoConnect(d networkmanager.Device, val bool) error {
	autoConnect, err := d.Device().Autoconnect().Get(0)
	if err != nil {
		return err
	}

	if autoConnect != val {
		err = d.Device().Autoconnect().Set(0, val)
		if err != nil {
			return err
		}
	}
	return nil
}

func setDeviceManaged(d networkmanager.Device, val bool) error {
	managed, err := d.Device().Managed().Get(0)
	if err != nil {
		return err
	}

	if managed != val {
		err = d.Device().Managed().Set(0, val)
		if err != nil {
			return err
		}
	}
	return nil
}

func restartIPWatchD() {
	go func() {
		err := exec.Command("systemctl", "restart", "ipwatchd.service").Run()
		if err != nil {
			logger.Warning(err)
		}
	}()
}
