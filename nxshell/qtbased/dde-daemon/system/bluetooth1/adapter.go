// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"fmt"
	"os"
	"time"

	"github.com/godbus/dbus/v5"
	bluez "github.com/linuxdeepin/go-dbus-factory/system/org.bluez"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
)

type adapter struct {
	bt      *SysBluetooth
	core    bluez.HCI
	Address string

	Path                dbus.ObjectPath
	Name                string
	Alias               string
	Powered             bool
	Discovering         bool
	Discoverable        bool
	DiscoverableTimeout uint32
	// discovering timer, when time is up, stop discovering until start button is clicked next time
	discoveringTimer *time.Timer
	// 扫描完成
	discoveringFinished                 bool
	scanReadyToConnectDeviceTimeoutFlag bool
	// 自动回连完成标志位
	autoConnectFinished bool
	// waitDiscovery未使用
	// waitDiscovery                       bool
	poweredActionTime time.Time
}

var defaultDiscoveringTimeout = 30 * time.Second

func newAdapter(systemSigLoop *dbusutil.SignalLoop, objPath dbus.ObjectPath) (a *adapter) {
	a = &adapter{Path: objPath, autoConnectFinished: false}
	systemConn := systemSigLoop.Conn()
	a.core, _ = bluez.NewHCI(systemConn, objPath)
	a.core.InitSignalExt(systemSigLoop, true)
	a.connectProperties()
	a.Address, _ = a.core.Adapter().Address().Get(0)
	// a.waitDiscovery = true
	// 用于定时停止扫描
	a.discoveringTimer = time.AfterFunc(defaultDiscoveringTimeout, func() {
		logger.Debug("discovery time out, stop discovering")
		// NOTE: 扫描结束后不用备份设备，因为处理设备添加时就会同时备份设备。
		//Scan timeout
		a.discoveringFinished = true
		if err := a.core.Adapter().StopDiscovery(0); err != nil {
			logger.Warningf("stop discovery failed, err:%v", err)
		}
		_bt.prepareToConnectedDevice = ""
	})
	// stop timer at first
	a.discoveringTimer.Stop()
	// fix alias
	alias, _ := a.core.Adapter().Alias().Get(0)
	if alias == "first-boot-hostname" {
		hostname, err := os.Hostname()
		if err == nil {
			if hostname != "first-boot-hostname" {
				// reset alias
				err = a.core.Adapter().Alias().Set(0, "")
				if err != nil {
					logger.Warning(err)
				}
			}
		} else {
			logger.Warning("failed to get hostname:", err)
		}
	}

	a.Alias, _ = a.core.Adapter().Alias().Get(0)
	a.Name, _ = a.core.Adapter().Name().Get(0)
	a.Powered, _ = a.core.Adapter().Powered().Get(0)
	a.Discovering, _ = a.core.Adapter().Discovering().Get(0)
	a.Discoverable, _ = a.core.Adapter().Discoverable().Get(0)
	a.DiscoverableTimeout, _ = a.core.Adapter().DiscoverableTimeout().Get(0)
	return
}

func (a *adapter) destroy() {
	a.core.RemoveHandler(proxy.RemoveAllHandlers)
}

func (a *adapter) String() string {
	return fmt.Sprintf("adapter %s [%s]", a.Alias, a.Address)
}

func (a *adapter) notifyAdapterAdded() {
	logger.Info("AdapterAdded", a)
	err := _bt.service.Emit(_bt, "AdapterAdded", marshalJSON(a))
	if err != nil {
		logger.Warning(err)
	}
	_bt.updateState()
}

func (a *adapter) notifyAdapterRemoved() {
	logger.Info("AdapterRemoved", a)
	err := _bt.service.Emit(_bt, "AdapterRemoved", marshalJSON(a))
	if err != nil {
		logger.Warning(err)
	}
	_bt.updateState()
}

func (a *adapter) notifyPropertiesChanged() {
	err := _bt.service.Emit(_bt, "AdapterPropertiesChanged", marshalJSON(a))
	if err != nil {
		logger.Warning(err)
	}
	_bt.updateState()
}

func (a *adapter) connectProperties() {
	err := a.core.Adapter().Name().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		a.Name = value
		logger.Debugf("%s Name: %v", a, value)
		a.notifyPropertiesChanged()
	})
	if err != nil {
		logger.Warning(err)
	}

	err = a.core.Adapter().Alias().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		a.Alias = value
		logger.Debugf("%s Alias: %v", a, value)
		a.notifyPropertiesChanged()
	})
	if err != nil {
		logger.Warning(err)
	}

	err = a.core.Adapter().Powered().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		// 接口操作时效内，按接口配置状态配置蓝牙状态。
		if a.Powered != value && time.Since(a.poweredActionTime) < time.Second {
			logger.Debug("sync power status because dde power status opt is working")
			go a.core.Adapter().Powered().Set(0, a.Powered)
			return
		}
		a.Powered = value
		logger.Debugf("%s Powered: %v", a, value)
		if a.Powered {
			//err := a.core.Adapter().Discoverable().Set(0, _bt.config.Discoverable)
			//if err != nil {
			//	logger.Warningf("failed to set discoverable for %s: %v", a, err)
			//}
			go func() {
				err = a.core.Adapter().StopDiscovery(0)
				if err != nil {
					logger.Warningf("failed to stop discovery for %s: %v", a, err)
				}
				// a.waitDiscovery = true
				// in case auto connect to device failed, only when signal power on is received, try to auto connect device
				_bt.tryConnectPairedDevices(a.Path)
			}()
		} else {
			// if power off, stop discovering time out
			a.discoveringTimer.Stop()
			a.bt.syncCommonToBackupDevices(a.Path)
		}
		// Sleep for 1s and wait for bluez to set the attributes before sending the attribute change signal
		time.Sleep(1 * time.Second)
		a.notifyPropertiesChanged()
	})
	if err != nil {
		logger.Warning(err)
	}

	err = a.core.Adapter().Discovering().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		if a.Discovering != value {
			a.Discovering = value
			logger.Debugf("%s Discovering: %v", a, value)

			if value {
				a.bt.syncCommonToBackupDevices(a.Path)
				a.discoveringTimer.Reset(defaultDiscoveringTimeout)
			} else {
				// 停止扫描了，设备属性不会再更新了，于是更新设备到备份设备
				a.bt.updateBackupDevices(a.Path)
			}
			a.notifyPropertiesChanged()
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	err = a.core.Adapter().Discoverable().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		a.Discoverable = value
		logger.Debugf("%s Discoverable: %v", a, value)
		a.notifyPropertiesChanged()
	})
	if err != nil {
		logger.Warning(err)
	}

	err = a.core.Adapter().DiscoverableTimeout().ConnectChanged(func(hasValue bool, value uint32) {
		if !hasValue {
			return
		}
		a.DiscoverableTimeout = value
		logger.Debugf("%s DiscoverableTimeout: %v", a, value)
		a.notifyPropertiesChanged()
	})
	if err != nil {
		logger.Warning(err)
	}
}

func (a *adapter) startDiscovery() {
	a.discoveringFinished = false
	// 已经开始扫描 或 回连未结束 禁止开始扫描
	if a.Discovering || !a.autoConnectFinished {
		return
	}

	logger.Debugf("start discovery")
	err := a.core.Adapter().StartDiscovery(0)
	if err != nil {
		logger.Warningf("failed to start discovery for %s: %v", a, err)
	} else {
		logger.Debug("reset timer for stop scan")
		// a.waitDiscovery = false
		// start discovering success, reset discovering timer
		a.discoveringTimer.Reset(defaultDiscoveringTimeout)
	}
}
