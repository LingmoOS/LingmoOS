// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

const adapteraddress = "00:1A:7D:DA:71:13"
const deviceaddress = "00:1A:7D:DA:71:13/00:1A:7D:DA:71:11"
const testfile = "testfile"

var _deviceConfig = &deviceConfig{
	Icon:       "computer",
	Connected:  false,
	LatestTime: 0,
}

func Test_config(t *testing.T) {
	configAdapters := map[string]*adapterConfig{
		"00:1A:7D:DA:71:13": {
			Powered:      false,
			Discoverable: true,
		},
	}

	configDevices := map[string]*deviceConfig{
		"00:1A:7D:DA:71:13/00:1A:7D:DA:71:11": {
			Icon:       "computer",
			Connected:  false,
			LatestTime: 0,
		},
		"00:1A:7D:DA:71:13/00:1A:7D:DA:71:13": {
			Icon:       "computer",
			Connected:  false,
			LatestTime: 0,
		},
		"00:1A:7D:DA:71:13/0B:4C:03:C8:EA:7C": {
			Icon:       "",
			Connected:  false,
			LatestTime: 0,
		},
		"00:1A:7D:DA:71:13/10:D0:7A:B1:D3:2F": {
			Icon:       "",
			Connected:  false,
			LatestTime: 0,
		},
		"00:1A:7D:DA:71:13/10:E9:53:E9:EA:3C": {
			Icon:       "phone",
			Connected:  false,
			LatestTime: 0,
		},
	}

	c := &config{}
	c.core.SetConfigFile(testfile)
	logger.Info("load bluetooth config file:", c.core.GetConfigFile())
	c.Adapters = make(map[string]*adapterConfig)
	c.Devices = make(map[string]*deviceConfig)
	//c.Discoverable = true

	c.addAdapterConfig(adapteraddress)

	for address, configDevice := range configDevices {
		c.addConfigDevice(address, configDevice)
	}

	c.load()

	assert.Equal(t, c.Adapters, configAdapters)
	assert.Equal(t, c.Devices, configDevices)
	//assert.True(t, c.Discoverable)

	c.setAdapterConfigPowered(adapteraddress, false)
	assert.False(t, c.getAdapterConfigPowered(adapteraddress))

	c.setAdapterConfigPowered(adapteraddress, true)
	assert.True(t, c.getAdapterConfigPowered(adapteraddress))

	c.setConfigDeviceConnected(deviceaddress, _deviceConfig, true)
	assert.True(t, c.getDeviceConfigConnected(deviceaddress))

	c.setConfigDeviceConnected(deviceaddress, _deviceConfig, false)
	assert.False(t, c.getDeviceConfigConnected(deviceaddress))

	err := os.Remove(testfile)
	logger.Warning("Remove failed:", err)
}

func (c *config) addConfigDevice(address string, addDeviceconfig *deviceConfig) {
	if c.isDeviceConfigExist(address) {
		return
	}

	deviceInfo := newDeviceConfig()
	deviceInfo.Icon = addDeviceconfig.Icon
	deviceInfo.LatestTime = 0
	deviceInfo.Connected = addDeviceconfig.Connected

	c.Devices[address] = deviceInfo
	c.save()
}

func (c *config) setConfigDeviceConnected(address string, Deviceconfig *deviceConfig, connected bool) {
	dc, ok := c.getDeviceConfig(address)
	if !ok {
		return
	}

	dc.Connected = connected
	dc.Icon = Deviceconfig.Icon

	c.save()
}
