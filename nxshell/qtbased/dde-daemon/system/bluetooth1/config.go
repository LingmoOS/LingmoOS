// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"sort"
	"strings"
	"time"

	"github.com/davecgh/go-spew/spew"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/utils"
)

type config struct {
	core utils.Config

	Adapters map[string]*adapterConfig // use adapter hardware address as key
	Devices  map[string]*deviceConfig  // use adapter address/device address as key

	//Discoverable bool `json:"discoverable"`
}

var (
	adapterDefaultPowered      bool = false //蓝牙开关默认状态
	adapterDefaultDiscoverable bool = true  //设备可被发现状态
)

type adapterConfig struct {
	Powered      bool
	Discoverable bool
}

type deviceConfig struct {
	// add icon info to mark device type
	Icon string
	// device connect status
	Connected bool
	// record latest time to do compare with other devices
	LatestTime int64
}

// add address message
type deviceConfigWithAddress struct {
	Icon       string
	Connected  bool
	LatestTime int64
	Address    string
}

func newConfig() (c *config) {
	c = &config{}
	c.core.SetConfigFile("/var/lib/dde-daemon/bluetooth/config.json")
	logger.Info("load bluetooth config file:", c.core.GetConfigFile())
	c.Adapters = make(map[string]*adapterConfig)
	c.Devices = make(map[string]*deviceConfig)
	//c.Discoverable = true
	return
}

func (c *config) load() {
	err := c.core.Load(c)
	if err != nil {
		logger.Warning(err)
	}
	if logger.GetLogLevel() == log.LevelDebug {
		logger.Debugf("load config, adapters: %v, devices: %v", spew.Sdump(c.Adapters), spew.Sdump(c.Devices))
	}
}

func (c *config) save() {
	err := c.core.Save(c)
	if err != nil {
		logger.Warning(err)
	}
}

func newAdapterConfig() (ac *adapterConfig) {
	ac = &adapterConfig{Powered: adapterDefaultPowered, Discoverable: adapterDefaultDiscoverable}
	return
}

func (c *config) clearSpareConfig(b *SysBluetooth) {
	var adapterAddresses []string
	// key is adapter address
	var adapterDevicesMap = make(map[string][]*device)

	b.adaptersMu.Lock()
	for _, adapter := range b.adapters {
		adapterAddresses = append(adapterAddresses, adapter.Address)
	}
	b.adaptersMu.Unlock()

	for _, adapterAddr := range adapterAddresses {
		adapterDevicesMap[adapterAddr] = b.getAdapterDevices(adapterAddr)
	}

	c.core.Lock()
	// NOTE: 最好不要删除适配器的配置，因为有时候获取不到适配器，然后调用了本方法，就把适配器关闭了。
	// remove spare devices
	for addr := range c.Devices {
		addrParts := strings.SplitN(addr, "/", 2)
		if len(addrParts) != 2 {
			delete(c.Devices, addr)
			continue
		}
		adapterAddr := addrParts[0]
		deviceAddr := addrParts[1]

		devices := adapterDevicesMap[adapterAddr]
		var foundDevice bool
		for _, device := range devices {
			if device.Address == deviceAddr {
				foundDevice = true
				break
			}
		}

		if !foundDevice {
			delete(c.Devices, addr)
			continue
		}
	}

	c.core.Unlock()
	c.save()
}

func (c *config) addAdapterConfig(address string) {
	if c.isAdapterConfigExists(address) {
		return
	}

	c.core.Lock()
	c.Adapters[address] = newAdapterConfig()
	c.core.Unlock()
	c.save()
}

func (c *config) isAdapterConfigExists(address string) (ok bool) {
	c.core.Lock()
	defer c.core.Unlock()
	_, ok = c.Adapters[address]
	return
}

func (c *config) getAdapterConfigPowered(address string) (powered bool) {
	c.core.Lock()
	defer c.core.Unlock()
	if ac, ok := c.Adapters[address]; ok {
		return ac.Powered
	}
	return false
}

func (c *config) setAdapterConfigPowered(address string, powered bool) {
	c.core.Lock()
	if ac, ok := c.Adapters[address]; ok {
		ac.Powered = powered
	}
	c.core.Unlock()
	c.save()
}

func (c *config) getAdapterConfigDiscoverable(address string) (discoverable bool) {
	c.core.Lock()
	defer c.core.Unlock()
	if ac, ok := c.Adapters[address]; ok {
		return ac.Discoverable
	}
	return false
}

func (c *config) setAdapterConfigDiscoverable(address string, discoverable bool) {
	c.core.Lock()
	if ac, ok := c.Adapters[address]; ok {
		ac.Discoverable = discoverable
	}
	c.core.Unlock()
	c.save()
}

func newDeviceConfig() (ac *deviceConfig) {
	ac = &deviceConfig{Connected: false}
	return
}

func (c *config) isDeviceConfigExist(address string) (ok bool) {
	c.core.Lock()
	defer c.core.Unlock()
	_, ok = c.Devices[address]
	return
}

// add device detail info into config file
func (c *config) addDeviceConfig(addDevice *device) {
	// check if device exist
	if c.isDeviceConfigExist(addDevice.getAddress()) {
		return
	}
	c.core.Lock()
	// save device info
	deviceInfo := newDeviceConfig()
	deviceInfo.Icon = addDevice.Icon
	// connect status is set false as default,so device has not been connected yet
	deviceInfo.LatestTime = 0
	deviceInfo.Connected = addDevice.connected
	//add device info to devices map
	c.Devices[addDevice.getAddress()] = deviceInfo
	c.core.Unlock()
	c.save()
}

func (c *config) getDeviceConfig(address string) (dc *deviceConfig, ok bool) {
	c.core.Lock()
	defer c.core.Unlock()
	dc, ok = c.Devices[address]
	return
}

func (c *config) getDeviceConfigConnected(address string) (connected bool) {
	dc, ok := c.getDeviceConfig(address)
	if !ok {
		return
	}

	c.core.Lock()
	defer c.core.Unlock()
	return dc.Connected
}

func (c *config) setDeviceConfigConnected(device *device, connected bool) {
	if device == nil {
		return
	}
	dc, ok := c.getDeviceConfig(device.getAddress())
	if !ok {
		return
	}

	c.core.Lock()
	dc.Connected = connected
	// when status is connected, set connected status as true, update the latest time
	dc.Connected = connected
	dc.Icon = device.Icon
	if connected {
		dc.LatestTime = time.Now().Unix()
	}

	c.core.Unlock()

	c.save()
}

// 根据配置文件中的最后连接时间 LatestTime 排序设备列表，最后连接时间越近（大），位置越前。
func (c *config) softDevices(devices []*device) {
	c.core.Lock()
	defer c.core.Unlock()

	sort.SliceStable(devices, func(i, j int) bool {
		devI := devices[i]
		devJ := devices[j]
		cfgI := c.Devices[devI.getAddress()]
		cfgJ := c.Devices[devJ.getAddress()]
		var latestTimeI int64 = 0
		var latestTimeJ int64 = 0
		if cfgI != nil {
			latestTimeI = cfgI.LatestTime
		}
		if cfgJ != nil {
			latestTimeJ = cfgJ.LatestTime
		}
		// LatestTime 越大的越在前面，设备配置（cfgI，cfgJ）为 nil 的排在最后面。
		return latestTimeI > latestTimeJ
	})
}

var _iconPriorityMap = map[string]int{
	devIconInputMouse:      0, // 最高
	devIconInputKeyboard:   1,
	devIconInputTablet:     2,
	devIconInputGaming:     2,
	devIconAudioCard:       3,
	devIconPhone:           4,
	devIconComputer:        4,
	devIconCameraPhoto:     5,
	devIconCameraVideo:     5,
	devIconModem:           6,
	devIconNetworkWireless: 6,
	devIconPrinter:         7,
}

func getPriorityWithIcon(icon string) int {
	p, ok := _iconPriorityMap[icon]
	if ok {
		return p
	}
	return priorityLowest
}

// 最低优先级
const priorityLowest = 99
