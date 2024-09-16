// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/network1/nm"
)

func newWiredConnectionForDevice(id, uuid string, devPath dbus.ObjectPath, active bool) (cpath dbus.ObjectPath, err error) {
	logger.Debugf("new wired connection, id=%s, uuid=%s, devPath=%s", id, uuid, devPath)
	data := newWiredConnectionData(id, uuid, devPath)

	setSettingConnectionAutoconnect(data, true)
	cpath, err = nmAddConnection(data)
	if err != nil {
		return "/", err
	}
	if active {
		_, err = nmActivateConnection(cpath, devPath)
		if err != nil {
			logger.Warningf("failed to activate connection cpath: %v, devPath: %v, err: %v",
				cpath, devPath, err)
		}
	}
	return cpath, nil
}

func newWiredConnectionData(id, uuid string, devPath dbus.ObjectPath) (data connectionData) {
	data = make(connectionData)

	addSetting(data, nm.NM_SETTING_CONNECTION_SETTING_NAME)
	setSettingConnectionId(data, id)
	setSettingConnectionUuid(data, uuid)
	setSettingConnectionType(data, nm.NM_SETTING_WIRED_SETTING_NAME)

	initSettingSectionWired(data, devPath)

	initSettingSectionIpv4(data)
	initSettingSectionIpv6(data)
	return
}

func initSettingSectionWired(data connectionData, devPath dbus.ObjectPath) {
	addSetting(data, nm.NM_SETTING_WIRED_SETTING_NAME)
	setSettingWiredDuplex(data, "full")

	ifc := nmGetDeviceInterface(devPath)
	if ifc == "" {
		logger.Debug("cant get interface name, ignore name")
		return
	}
	setSettingConnectionInterfaceName(data, ifc)
	// need to set macAddress
	hwAddr, err := nmGeneralGetDeviceHwAddr(devPath, true)
	if err == nil {
		setSettingWiredMacAddress(data, convertMacAddressToArrayByte(hwAddr))
	}
}
