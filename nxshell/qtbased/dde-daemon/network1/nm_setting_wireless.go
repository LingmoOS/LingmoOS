// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"os"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/network1/nm"
)

func newWirelessHotspotConnectionForDevice(id, uuid string, devPath dbus.ObjectPath, active bool) (cpath dbus.ObjectPath, err error) {
	logger.Infof("new wireless hotspot connection, id=%s, uuid=%s, devPath=%s", id, uuid, devPath)
	data := newWirelessHotspotConnectionData(id, uuid)
	setSettingConnectionInterfaceName(data, nmGetDeviceInterface(devPath))
	setSettingWirelessSsid(data, []byte(os.Getenv("USER")))
	setSettingWirelessSecurityKeyMgmt(data, "none")
	hwAddr, _ := nmGeneralGetDeviceHwAddr(devPath, true)
	setSettingWirelessMacAddress(data, convertMacAddressToArrayByte(hwAddr))
	if active {
		cpath, _, err = nmAddAndActivateConnection(data, devPath, true)
	} else {
		cpath, err = nmAddConnection(data)
	}
	return
}

// new connection data
func newWirelessConnectionData(id, uuid string, ssid []byte, keymgmt, macAddress string) (data connectionData) {
	logger.Debug("newWirelessConnectionData: keymgmt:", keymgmt)
	data = make(connectionData)

	addSetting(data, nm.NM_SETTING_CONNECTION_SETTING_NAME)
	setSettingConnectionId(data, id)
	setSettingConnectionUuid(data, uuid)
	setSettingConnectionType(data, nm.NM_SETTING_WIRELESS_SETTING_NAME)

	addSetting(data, nm.NM_SETTING_WIRELESS_SETTING_NAME)
	if ssid != nil {
		setSettingWirelessSsid(data, ssid)
	}
	setSettingWirelessMode(data, nm.NM_SETTING_WIRELESS_MODE_INFRA)

	err := logicSetSettingVkWirelessSecurityKeyMgmt(data, keymgmt)
	if err != nil {
		logger.Debug("failed to set VKWirelessSecutiryKeyMgmt")
		return
	}

	if macAddress != "" {
		setSettingWirelessMacAddress(data, convertMacAddressToArrayByte(macAddress))
	}

	initSettingSectionIpv4(data)
	initSettingSectionIpv6(data)

	return
}

func newWirelessHotspotConnectionData(id, uuid string) (data connectionData) {
	data = newWirelessConnectionData(id, uuid, nil, "none", "")
	err := logicSetSettingWirelessMode(data, nm.NM_SETTING_WIRELESS_MODE_AP)
	if err != nil {
		logger.Debug("failed to set WirelessMode")
		return
	}
	setSettingConnectionAutoconnect(data, false)
	return
}

// Logic setter
func logicSetSettingWirelessMode(data connectionData, value string) (err error) {
	// for ad-hoc or ap-hotspot mode, wpa-eap security is invalid, and
	// set ip4 method to "shared"
	if value != nm.NM_SETTING_WIRELESS_MODE_INFRA {
		if getSettingVkWirelessSecurityKeyMgmt(data) == "wpa-eap" {
			err = logicSetSettingVkWirelessSecurityKeyMgmt(data, "wpa-psk")
			if err != nil {
				logger.Debug("failed to set VkWirelessKeyMgmt")
				return err
			}
		}
		setSettingIP4ConfigMethod(data, nm.NM_SETTING_IP4_CONFIG_METHOD_SHARED)
	}
	setSettingWirelessMode(data, value)
	return
}
