// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network1

import (
	"testing"

	dbus "github.com/godbus/dbus/v5"
	"github.com/stretchr/testify/assert"
)

func Test_IsDeviceEnabled(t *testing.T) {
	n := Network{
		config: &Config{
			VpnEnabled: false,
			Devices: map[string]*DeviceConfig{
				"test": {
					Enabled: true,
				},
			},
		},
		devices: map[dbus.ObjectPath]*device{
			"1": {
				iface: "iFace",
				type0: 0,
			},
		},
	}

	enable, err := n.IsDeviceEnabled("iFace")
	assert.True(t, enable)
	assert.Nil(t, err)

	enable, err = n.IsDeviceEnabled("iFace1")
	assert.False(t, enable)
	assert.NotNil(t, err)
}

func Test_isDeviceEnabled(t *testing.T) {
	n := Network{
		config: &Config{
			VpnEnabled: false,
			Devices: map[string]*DeviceConfig{
				"test": {
					Enabled: true,
				},
			},
		},
		devices: map[dbus.ObjectPath]*device{
			"1": {
				iface: "iFace",
				type0: 0,
			},
		},
	}

	enable, err := n.isDeviceEnabled("iFace")
	assert.True(t, enable)
	assert.Nil(t, err)

	enable, err = n.isDeviceEnabled("iFace1")
	assert.False(t, enable)
	assert.NotNil(t, err)
}

func Test_findDevice(t *testing.T) {
	n := Network{}
	n.findDevice("/org/freedesktop/NetworkManager")

	n.findDevice("test")
}

func Test_getDeviceByIface(t *testing.T) {
	n := Network{}
	device1 := n.getDeviceByIface("test")
	assert.Nil(t, device1)

	n = Network{
		devices: map[dbus.ObjectPath]*device{
			"1": {
				iface: "test",
				type0: 0,
			},
		},
	}

	device1 = n.getDeviceByIface("test")
	assert.Equal(t, uint32(0), device1.type0)
}

func Test_saveConfig_Network(t *testing.T) {
	n := Network{}
	n.saveConfig()
}
