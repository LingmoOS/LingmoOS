// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network1

import (
	"github.com/godbus/dbus/v5"
	nmdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.networkmanager"
)

type connectionData map[string]map[string]dbus.Variant

func newWiredConnectionData(id, uuid string, devPath dbus.ObjectPath) (data connectionData) {
	data = make(connectionData)

	addSetting(data, "connection")
	setSettingConnectionId(data, id)
	setSettingConnectionUuid(data, uuid)
	setSettingConnectionType(data, "802-3-ethernet")

	// dont set mac
	initSettingSectionWired(data, devPath)

	initSettingSectionIpv4(data)
	initSettingSectionIpv6(data)
	return
}

func initSettingSectionIpv4(data connectionData) {
	addSetting(data, "ipv4")
	setSettingIP4ConfigMethod(data, "auto")
	setSettingIP4ConfigNeverDefault(data, false)
}

func initSettingSectionWired(data connectionData, devPath dbus.ObjectPath) {
	addSetting(data, "802-3-ethernet")
	setSettingWiredDuplex(data, "full")

	ifc := nmGetDeviceInterface(devPath)
	if ifc == "" {
		logger.Debug("cant get interface name, ignore name")
		return
	}
	setSettingConnectionInterfaceName(data, ifc)
}

func setSettingConnectionInterfaceName(data connectionData, value string) {
	setSettingKey(data, "connection", "interface-name", value)
}

func setSettingWiredDuplex(data connectionData, value string) {
	setSettingKey(data, "802-3-ethernet", "duplex", value)
}

func setSettingConnectionType(data connectionData, value string) {
	setSettingKey(data, "connection", "type", value)
}

func setSettingConnectionUuid(data connectionData, value string) {
	setSettingKey(data, "connection", "uuid", value)
}

func addSetting(data connectionData, setting string) {
	var settingData map[string]dbus.Variant
	_, ok := data[setting]
	if !ok {
		// add setting if not exists
		settingData = make(map[string]dbus.Variant)
		data[setting] = settingData
	}
}

func setSettingConnectionId(data connectionData, value string) {
	setSettingKey(data, "connection", "id", value)
}

func setSettingKey(data connectionData, section, key string, value interface{}) {
	var sectionData map[string]dbus.Variant
	sectionData, ok := data[section]
	if !ok {
		logger.Errorf(`set connection data failed, section "%s" is not exits yet`, section)
		return
	}
	sectionData[key] = dbus.MakeVariant(value)
}

func setSettingIP4ConfigMethod(data connectionData, value string) {
	setSettingKey(data, "ipv4", "method", value)
}

func initSettingSectionIpv6(data connectionData) {
	addSetting(data, "ipv6")
	setSettingIP6ConfigMethod(data, "auto")
}

func setSettingIP4ConfigNeverDefault(data connectionData, value bool) {
	setSettingKey(data, "ipv4", "never-default", value)
}

func setSettingIP6ConfigMethod(data connectionData, value string) {
	setSettingKey(data, "ipv6", "method", value)
}

func nmGetDeviceInterface(devPath dbus.ObjectPath) (devInterface string) {
	d, err := nmNewDevice(devPath)
	if err != nil {
		return
	}

	dev := d.Device()
	devInterface, _ = dev.Interface().Get(0)
	return
}

func nmNewDevice(devPath dbus.ObjectPath) (dev nmdbus.Device, err error) {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}
	dev, err = nmdbus.NewDevice(systemBus, devPath)
	if err != nil {
		logger.Error(err)
		return
	}
	return
}
