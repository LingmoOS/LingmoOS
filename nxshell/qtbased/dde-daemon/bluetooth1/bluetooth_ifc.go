// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"encoding/json"
	"errors"
	"fmt"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

func (b *Bluetooth) ConnectDevice(device dbus.ObjectPath, apath dbus.ObjectPath) *dbus.Error {
	logger.Infof("dbus call ConnectDevice with device %v and apath %v", device, apath)

	b.setInitiativeConnect(device, true)
	err := b.sysBt.ConnectDevice(0, device, apath)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) DisconnectDevice(device dbus.ObjectPath) *dbus.Error {
	logger.Infof("dbus call DisconnectDevice with device %v", device)

	err := b.sysBt.DisconnectDevice(0, device)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) RemoveDevice(adapter, device dbus.ObjectPath) *dbus.Error {
	logger.Infof("dbus call RemoveDevice with adapter %v and device %v", adapter, device)

	err := b.sysBt.RemoveDevice(0, adapter, device)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) SetDeviceAlias(device dbus.ObjectPath, alias string) *dbus.Error {
	logger.Infof("dbus call SetDeviceAlias with device %v and alias %s",
		device, alias)

	err := b.sysBt.SetDeviceAlias(0, device, alias)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) SetDeviceTrusted(device dbus.ObjectPath, trusted bool) *dbus.Error {
	logger.Infof("dbus call SetDeviceTrusted with device %v and trusted %t",
		device, trusted)

	err := b.sysBt.SetDeviceTrusted(0, device, trusted)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

// GetDevices return all device objects that marshaled by json.
func (b *Bluetooth) GetDevices(adapter dbus.ObjectPath) (devicesJSON string, busErr *dbus.Error) {
	logger.Infof("dbus call GetDevices with adapter %v", adapter)

	devices := b.devices.getDevices(adapter)
	devicesJson := marshalJSON(devices)
	return devicesJson, nil
}

// GetAdapters return all adapter objects that marshaled by json.
func (b *Bluetooth) GetAdapters() (adaptersJSON string, busErr *dbus.Error) {
	logger.Info("dbus call GetAdapters")
	return b.adapters.toJSON(), nil
}

func (b *Bluetooth) RequestDiscovery(adapter dbus.ObjectPath) *dbus.Error {
	logger.Infof("dbus call RequestDiscovery with adapter %v ", adapter)

	err := b.sysBt.RequestDiscovery(0, adapter)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

// SendFiles 用来发送文件给蓝牙设备，仅支持发送给已连接设备
func (b *Bluetooth) SendFiles(devAddress string, files []string) (sessionPath dbus.ObjectPath, busErr *dbus.Error) {
	logger.Infof("dbus call SendFiles with devAddress %s and files %v", devAddress, files)

	if len(files) == 0 {
		err := errors.New("files is empty")
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	if !b.CanSendFile {
		err := errors.New("no permission")
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	// 检查设备是否已经连接
	dev := b.getConnectedDeviceByAddress(devAddress)
	if dev == nil {
		err := errors.New("device not connected")
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	sessionPath, err := b.sendFiles(dev, files)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	return sessionPath, nil
}

// CancelTransferSession 用来取消发送的会话，将会终止会话中所有的传送任务
func (b *Bluetooth) CancelTransferSession(sessionPath dbus.ObjectPath) *dbus.Error {
	logger.Infof("dbus call CancelTransferSession with sessionPath %v", sessionPath)

	//添加延时，确保sessionPath被remove，防止死锁
	time.Sleep(500 * time.Millisecond)
	b.sessionCancelChMapMu.Lock()
	defer b.sessionCancelChMapMu.Unlock()

	cancelCh, ok := b.sessionCancelChMap[sessionPath]
	if !ok {
		err := errors.New("session not exists")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	cancelCh <- struct{}{}

	return nil
}

func (b *Bluetooth) SetAdapterPowered(adapter dbus.ObjectPath,
	powered bool) *dbus.Error {
	logger.Infof("dbus call SetAdapterPowered with adapter %v and powered %t",
		adapter, powered)
	// 当蓝牙开关打开时，需要同步session蓝牙中devices
	if powered {
		devicesJSON, err := b.sysBt.GetDevices(0, adapter)
		if err == nil {
			var devices DeviceInfos
			err = json.Unmarshal([]byte(devicesJSON), &devices)
			if err == nil {
				b.devices.mu.Lock()
				b.devices.infos[adapter] = devices
				b.devices.mu.Unlock()
			} else {
				logger.Warning(err)
			}

		} else {
			logger.Warning(err)
		}
	} else {
		err := b.handleBluezPort(powered)
		if err != nil {
			logger.Warning(err)
		}
	}

	err := b.sysBt.SetAdapterPowered(0, adapter, powered)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) SetAdapterAlias(adapter dbus.ObjectPath, alias string) *dbus.Error {
	logger.Infof("dbus call SetAdapterAlias with adapter %v and alias %s", adapter, alias)

	err := b.sysBt.SetAdapterAlias(0, adapter, alias)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) SetAdapterDiscoverable(adapter dbus.ObjectPath,
	discoverable bool) *dbus.Error {
	logger.Infof("dbus call SetAdapterDiscoverable with adapter %v and discoverable %t",
		adapter, discoverable)

	err := b.sysBt.SetAdapterDiscoverable(0, adapter, discoverable)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) SetAdapterDiscovering(adapter dbus.ObjectPath,
	discovering bool) *dbus.Error {
	logger.Infof("dbus call SetAdapterDiscovering with adapter %v and discovering %t",
		adapter, discovering)

	err := b.sysBt.SetAdapterDiscovering(0, adapter, discovering)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) SetAdapterDiscoverableTimeout(adapter dbus.ObjectPath,
	discoverableTimeout uint32) *dbus.Error {
	logger.Infof("dbus call SetAdapterDiscoverableTimeout with adapter %v and discoverableTimeout %d",
		adapter, discoverableTimeout)

	err := b.sysBt.SetAdapterDiscoverableTimeout(0, adapter, discoverableTimeout)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

// Confirm should call when you receive RequestConfirmation signal
func (b *Bluetooth) Confirm(device dbus.ObjectPath, accept bool) *dbus.Error {
	logger.Infof("dbus call Confirm with device %v and accept %t", device, accept)

	err := b.feed(device, accept, "")
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

// FeedPinCode should call when you receive RequestPinCode signal, notice that accept must true
// if you accept connect request. If accept is false, pinCode will be ignored.
func (b *Bluetooth) FeedPinCode(device dbus.ObjectPath, accept bool, pinCode string) *dbus.Error {
	logger.Infof("dbus call FeedPinCode with device %v, accept %t and pinCode %s", device, accept, pinCode)

	err := b.feed(device, accept, pinCode)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

// FeedPasskey should call when you receive RequestPasskey signal, notice that accept must true
// if you accept connect request. If accept is false, passkey will be ignored.
// passkey must be range in 0~999999.
func (b *Bluetooth) FeedPasskey(device dbus.ObjectPath, accept bool, passkey uint32) *dbus.Error {
	logger.Infof("dbus call FeedPasskey with device %v, accept %t and passkey %d", device, accept, passkey)

	err := b.feed(device, accept, fmt.Sprintf("%06d", passkey))
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (b *Bluetooth) DebugInfo() (info string, busErr *dbus.Error) {
	logger.Info("dbus call DebugInfo")

	info, err := b.sysBt.DebugInfo(0)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	return info, nil
}

// ClearUnpairedDevice will remove all device in unpaired list
func (b *Bluetooth) ClearUnpairedDevice() *dbus.Error {
	logger.Infof("dbus call ClearUnpairedDevice")

	err := b.sysBt.ClearUnpairedDevice(0)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}
