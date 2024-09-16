// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"fmt"
	"path/filepath"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/keyfile"
)

const (
	bluetoothPrefixDir = "/var/lib/bluetooth"

	kfSectionGeneral  = "General"
	kfKeyTechnologies = "SupportedTechnologies"
)

func (*Daemon) BluetoothGetDeviceTechnologies(adapter, device string) (technologies []string, busErr *dbus.Error) {
	var filename = filepath.Join(bluetoothPrefixDir, adapter, device, "info")
	if !filepath.HasPrefix(filename, bluetoothPrefixDir) {
		return nil, dbusutil.ToError(fmt.Errorf("invaild adapter: [%s] device: [%s]", adapter, device))
	}
	technologies, err := doBluetoothGetDeviceTechnologies(filename)
	if err != nil {
		return nil, dbusutil.ToError(err)
	}
	return technologies, nil
}

func doBluetoothGetDeviceTechnologies(filename string) ([]string, error) {
	var kf = keyfile.NewKeyFile()
	err := kf.LoadFromFile(filename)
	if err != nil {
		return nil, fmt.Errorf("can't parse file: %s", filename)
	}
	return kf.GetStringList(kfSectionGeneral, kfKeyTechnologies)
}
