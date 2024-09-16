// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"errors"
	"fmt"
	"strings"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/network1/nm"
	mmdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.modemmanager1"
	nmdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.networkmanager"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

type device struct {
	nmDev      nmdbus.Device
	mmDevModem mmdbus.Modem
	nmDevType  uint32
	id         string
	Udi        string

	Path          dbus.ObjectPath
	State         uint32
	Interface     string
	ClonedAddress string
	HwAddress     string
	Driver        string
	Managed       bool

	// Vendor is the device vendor ID and product ID, if failed, use
	// interface name instead. BTW, we use Vendor instead of
	// Description as the name to keep compatible with the old
	// front-end code.
	Vendor string

	// Unique connection uuid for this device, works for wired,
	// wireless and modem devices, for wireless device the unique uuid
	// will be the connection uuid of hotspot mode.
	UniqueUuid string

	UsbDevice bool // not works for mobile device(modem)

	// used for wireless device
	ActiveAp       dbus.ObjectPath
	SupportHotspot bool
	Mode           uint32

	// used for mobile device
	MobileNetworkType   string
	MobileSignalQuality uint32

	InterfaceFlags uint32
}

const (
	nmInterfaceFlagUP uint32 = 1 << iota
)

func (m *Manager) initDeviceManage() {
	m.devicesLock.Lock()
	m.devices = make(map[string][]*device)
	m.devicesLock.Unlock()

	m.accessPointsLock.Lock()
	m.accessPoints = make(map[dbus.ObjectPath][]*accessPoint)
	m.accessPointsLock.Unlock()

	_, err := nmManager.ConnectDeviceAdded(func(path dbus.ObjectPath) {
		m.addDevice(path)
	})
	if err != nil {
		logger.Warning(err)
	}
	_, err = nmManager.ConnectDeviceRemoved(func(path dbus.ObjectPath) {
		notifyDeviceRemoved(path)
		m.removeDevice(path)
	})
	if err != nil {
		logger.Warning(err)
	}
	for _, path := range nmGetDevices() {
		m.addDevice(path)
	}
}

// 调整nmDevice的状态
func (m *Manager) adjustDeviceStatus() {
	for _, path := range nmGetDevices() {
		nmDev, err := nmNewDevice(path)
		if err != nil {
			continue
		}

		// ignore virtual network interfaces
		if isVirtualDeviceIfc(nmDev) {
			continue
		}

		// 如果nmDevice在network启动之前就已经是密码待验证的状态，需要等待nm超时才能自动回连，导致wifi回连太慢
		// 在这种情况下主动回连
		stat, _ := nmDev.Device().State().Get(0)
		if stat == nm.NM_DEVICE_STATE_NEED_AUTH {
			err := m.enableDevice(path, true, true)
			if err != nil {
				logger.Warning("failed to enable device:", err)
			}
		}
	}
}

func (m *Manager) newDevice(devPath dbus.ObjectPath) (dev *device, err error) {
	nmDev, err := nmNewDevice(devPath)
	if err != nil {
		return
	}

	// ignore virtual network interfaces
	if isVirtualDeviceIfc(nmDev) {
		driver, _ := nmDev.Device().Driver().Get(0)
		err = fmt.Errorf("ignore virtual network interface which driver is %s %s", driver, devPath)
		logger.Info(err)
		return
	}

	devType, _ := nmDev.Device().DeviceType().Get(0)
	if !isDeviceTypeValid(devType) {
		err = fmt.Errorf("ignore invalid device type %d", devType)
		logger.Info(err)
		return
	}

	dev = &device{
		nmDev:     nmDev,
		nmDevType: devType,
		Path:      nmDev.Path_(),
	}
	dev.Udi, _ = nmDev.Device().Udi().Get(0)
	dev.Driver, _ = nmDev.Device().Driver().Get(0)

	dev.Vendor = nmGeneralGetDeviceDesc(devPath)
	dev.UsbDevice = nmGeneralIsUsbDevice(devPath)
	dev.id, _ = nmGeneralGetDeviceIdentifier(devPath)
	dev.UniqueUuid = nmGeneralGetDeviceUniqueUuid(devPath)

	nmDev.InitSignalExt(m.sysSigLoop, true)

	// dispatch for different device types
	switch dev.nmDevType {
	case nm.NM_DEVICE_TYPE_ETHERNET:
		// for mac address clone
		nmDevWired := nmDev.Wired()
		err = nmDevWired.HwAddress().ConnectChanged(func(hasValue bool, value string) {
			if !hasValue {
				return
			}
			if value == dev.ClonedAddress {
				return
			}
			dev.ClonedAddress = value
			m.updatePropDevices()
		})
		if err != nil {
			logger.Warning(err)
		}
		dev.ClonedAddress, _ = nmDevWired.HwAddress().Get(0)
		dev.HwAddress, _ = nmDevWired.PermHwAddress().Get(0)

		if dev.HwAddress == "" {
			dev.HwAddress = dev.ClonedAddress
		}

		// wired device should always create non-tmp connections, nm will create tmp connection sometimes when device first plugin in
		err = nmDev.Device().ActiveConnection().ConnectChanged(func(hasValue bool, value dbus.ObjectPath) {
			// if has not value or value is not expected, ignore changes
			if !hasValue || value == "/" || value == "" {
				return
			}
			// try to get active connection
			_, _, err = m.ensureWiredConnectionExists(devPath, true)
			if err != nil {
				logger.Warningf("ensure wired connection failed, err: %v", err)
			}
		})
		if err != nil {
			logger.Warningf("connect to ActivateConnection failed, err: %v", err)
		}
	case nm.NM_DEVICE_TYPE_WIFI:
		nmDevWireless := nmDev.Wireless()
		dev.ClonedAddress, _ = nmDevWireless.HwAddress().Get(0)
		dev.HwAddress, _ = nmDevWireless.PermHwAddress().Get(0)

		// connect property, about wireless active access point
		err = nmDevWireless.ActiveAccessPoint().ConnectChanged(func(hasValue bool,
			value dbus.ObjectPath) {
			if !hasValue {
				return
			}
			if !m.isDeviceExists(devPath) {
				return
			}
			m.devicesLock.Lock()
			defer m.devicesLock.Unlock()
			dev.ActiveAp = value
			m.updatePropDevices()

			// when wifi is roaming, wpa and network-manager will deal this situation,
			// dde dont need try to re active connection or may cause error connection in OPT env
			// this case always means nm or wpa has bug, fix nm or wpa is better
		})
		if err != nil {
			logger.Warning(err)
		}
		dev.ActiveAp, _ = nmDevWireless.ActiveAccessPoint().Get(0)

		err = nmDevWireless.Mode().ConnectChanged(func(hasValue bool, value uint32) {
			if !hasValue {
				return
			}
			m.devicesLock.Lock()
			defer m.devicesLock.Unlock()
			dev.Mode = value
			m.updatePropDevices()
		})
		if err != nil {
			logger.Warning(err)
		}
		dev.Mode, _ = nmDevWireless.Mode().Get(0)

		permHwAddress, _ := nmDevWireless.PermHwAddress().Get(0)
		dev.SupportHotspot = isWirelessDeviceSupportHotspot(permHwAddress)

		if !dev.SupportHotspot {
			//WirelessCapabilities: The capabililities of wireless device 此属性包含了wifi网卡的所有特性
			//根据nm手册和源码，此处判断网卡是否支持ap模式修改为此方式判断
			wirelessCapabilities, _ := nmDevWireless.WirelessCapabilities().Get(0)
			if wirelessCapabilities&nm.NM_WIFI_DEVICE_CAP_AP != 0 {
				dev.SupportHotspot = true
			}
		}

		err = nmDevWireless.HwAddress().ConnectChanged(func(hasValue bool, value string) {
			if !hasValue {
				return
			}
			if value == dev.ClonedAddress {
				return
			}
			dev.ClonedAddress = value
			m.updatePropDevices()
		})
		if err != nil {
			logger.Warning(err)
		}

		logger.Debug("[newDevice] dsg LoadServiceFromNM : ", m.loadServiceFromNM)
		if !m.loadServiceFromNM {
			err = nmDevWireless.AccessPoints().ConnectChanged(func(hasValue bool, value []dbus.ObjectPath) {
				if !hasValue {
					return
				}

				m.accessPointsLock.Lock()
				shouldRemove := make([]dbus.ObjectPath, 0, len(m.accessPoints[devPath]))
				for _, a := range m.accessPoints[devPath] {
					var found bool
					for _, v := range value {
						if v == a.Path {
							found = true
							break
						}
					}

					if !found {
						shouldRemove = append(shouldRemove, a.Path)
					}
				}

				shouldAdd := make([]dbus.ObjectPath, 0, len(value))
				for _, v := range value {
					var found bool
					for _, a := range m.accessPoints[devPath] {
						if v == a.Path {
							found = true
							break
						}
					}

					if !found {
						shouldAdd = append(shouldAdd, v)
					}
				}

				for _, a := range shouldRemove {
					m.removeAccessPoint(devPath, a)
				}

				for _, a := range shouldAdd {
					m.addAccessPoint(devPath, a)
				}

				m.PropsMu.Lock()
				m.updatePropWirelessAccessPoints()
				m.PropsMu.Unlock()
				m.accessPointsLock.Unlock()
			})
			if err != nil {
				logger.Warning("connect to AccessPoints changed failed:", err)
			}
		}

		accessPoints := nmGetAccessPoints(devPath)
		m.initAccessPoints(dev.Path, accessPoints)

		m.WirelessAccessPoints, _ = marshalJSON(m.accessPoints)

	case nm.NM_DEVICE_TYPE_MODEM:
		if len(dev.id) == 0 {
			// some times, modem device will not be identified
			// successful for battery issue, so check and ignore it
			// here
			err = fmt.Errorf("modem device is not properly identified, please re-plugin it")
			return
		}
		go func() {
			// disable autoconnect property for mobile devices
			// notice: sleep is necessary seconds before setting dbus values
			// FIXME: seems network-manager will restore Autoconnect's value some times
			time.Sleep(3 * time.Second)
			nmSetDeviceAutoconnect(dev.Path, false)
		}()
		if mmDevModem, err := mmNewModem(dbus.ObjectPath(dev.Udi)); err == nil {
			mmDevModem.InitSignalExt(m.sysSigLoop, true)
			dev.mmDevModem = mmDevModem

			// connect properties
			err = dev.mmDevModem.Modem().AccessTechnologies().ConnectChanged(func(hasValue bool,
				value uint32) {
				if !m.isDeviceExists(devPath) {
					return
				}
				if !hasValue {
					return
				}
				m.devicesLock.Lock()
				defer m.devicesLock.Unlock()
				dev.MobileNetworkType = mmDoGetModemMobileNetworkType(value)
				m.updatePropDevices()
			})
			if err != nil {
				logger.Warning(err)
			}
			accessTech, _ := mmDevModem.Modem().AccessTechnologies().Get(0)
			dev.MobileNetworkType = mmDoGetModemMobileNetworkType(accessTech)

			err = dev.mmDevModem.Modem().SignalQuality().ConnectChanged(func(hasValue bool,
				value mmdbus.ModemSignalQuality) {
				if !m.isDeviceExists(devPath) {
					return
				}
				if !hasValue {
					return
				}

				m.devicesLock.Lock()
				defer m.devicesLock.Unlock()
				dev.MobileSignalQuality = value.Quality
				m.updatePropDevices()
			})
			if err != nil {
				logger.Warning(err)
			}
			dev.MobileSignalQuality = mmDoGetModemDeviceSignalQuality(mmDevModem)
		}
	}

	// connect signals
	_, err = dev.nmDev.Device().ConnectStateChanged(func(newState uint32, oldState uint32, reason uint32) {
		logger.Debugf("device %s state changed, %d => %d, reason[%d] %s",
			string(devPath), oldState, newState, reason, deviceErrorTable[reason])

		if !m.isDeviceExists(devPath) {
			return
		}

		if newState == nm.NM_DEVICE_STATE_ACTIVATED {
			// 网络链接状态更改  重置Portal认证状态
			m.protalAuthBrowserOpened = false
		}

		dev.State = newState
		m.devicesLock.Lock()
		m.updatePropDevices()
		m.devicesLock.Unlock()

	})
	if err != nil {
		logger.Warning(err)
	}

	err = dev.nmDev.Device().Interface().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}

		dev.Interface = value
		m.devicesLock.Lock()
		m.updatePropDevices()
		m.devicesLock.Unlock()
	})
	if err != nil {
		logger.Warning(err)
	}

	err = dev.nmDev.Device().Managed().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}

		dev.Managed = value
		m.devicesLock.Lock()
		m.updatePropDevices()
		m.devicesLock.Unlock()
	})
	if err != nil {
		logger.Warning(err)
	}

	dev.State, _ = nmDev.Device().State().Get(0)
	dev.Interface, _ = nmDev.Device().Interface().Get(0)
	// get device enable state from system network
	enabled, _ := m.sysNetwork.IsDeviceEnabled(0, dev.Interface)
	// adjust device enable state
	// due to script in pre-up and pre-down, device will be always set as true or false,
	// in this situation, the config kept in local file is not exact, device state should be adjust
	if isDeviceStateInActivating(dev.State) && !enabled {
		err = dev.nmDev.Device().Disconnect(0)
		if err != nil {
			logger.Warningf("device disconnected failed, err: %v", err)
		}
	} else if enabled {
		err = dev.nmDev.Device().Autoconnect().Set(0, true)
		if err != nil {
			logger.Warningf("init set auto-connect state failed, err: %v", err)
		}
	}
	dev.Managed = nmGeneralIsDeviceManaged(devPath)

	// get interface-flags
	dev.InterfaceFlags, err = nmDev.Device().InterfaceFlags().Get(0)
	if err != nil {
		logger.Warningf("get interface-flags failed, err: %v", err)
	} else {
		logger.Debugf("get interface-flags success, flags: %v", dev.InterfaceFlags)
	}
	// monitor interface-flags changed signal
	err = dev.nmDev.Device().InterfaceFlags().ConnectChanged(func(hasValue bool, value uint32) {
		if !hasValue {
			return
		}
		logger.Debugf("interface-flags changed, flags: %v", value)
		dev.InterfaceFlags = value
		m.updatePropDevices()
	})
	if err != nil {
		logger.Warningf("connected interface-flags failed, err: %v", err)
	}

	return
}

func (m *Manager) destroyDevice(dev *device) {
	// destroy object to reset all property connects
	if dev.mmDevModem != nil {
		mmDestroyModem(dev.mmDevModem)
	}
	nmDestroyDevice(dev.nmDev)
}

func (m *Manager) clearDevices() {
	m.devicesLock.Lock()
	defer m.devicesLock.Unlock()
	for _, devs := range m.devices {
		for _, dev := range devs {
			m.destroyDevice(dev)
		}
	}
	m.devices = make(map[string][]*device)
	m.updatePropDevices()
}

func (m *Manager) addDevice(devPath dbus.ObjectPath) {
	if m.isDeviceExists(devPath) {
		logger.Warning("device already exists", devPath)
		return
	}

	m.devicesLock.Lock()
	defer m.devicesLock.Unlock()
	dev, err := m.newDevice(devPath)
	if err != nil {
		return
	}

	logger.Debug("add device", devPath)
	devType := getCustomDeviceType(dev.nmDevType)
	m.devices[devType] = append(m.devices[devType], dev)
	m.updatePropDevices()
}

func (m *Manager) removeDevice(devPath dbus.ObjectPath) {
	if !m.isDeviceExists(devPath) {
		return
	}
	devType, i := m.getDeviceIndex(devPath)

	m.devicesLock.Lock()
	defer m.devicesLock.Unlock()
	m.devices[devType] = m.doRemoveDevice(m.devices[devType], i)
	m.updatePropDevices()
}
func (m *Manager) doRemoveDevice(devs []*device, i int) []*device {
	logger.Infof("remove device %#v", devs[i])
	m.destroyDevice(devs[i])
	copy(devs[i:], devs[i+1:])
	devs[len(devs)-1] = nil
	devs = devs[:len(devs)-1]
	return devs
}

func (m *Manager) getDevice(devPath dbus.ObjectPath) (dev *device) {
	devType, i := m.getDeviceIndex(devPath)
	if i < 0 {
		return
	}

	m.devicesLock.Lock()
	defer m.devicesLock.Unlock()
	return m.devices[devType][i]
}
func (m *Manager) isDeviceExists(devPath dbus.ObjectPath) bool {
	_, i := m.getDeviceIndex(devPath)
	return i >= 0
}
func (m *Manager) getDeviceIndex(devPath dbus.ObjectPath) (devType string, index int) {
	m.devicesLock.Lock()
	defer m.devicesLock.Unlock()
	for t, devs := range m.devices {
		for i, dev := range devs {
			if dev.Path == devPath {
				return t, i
			}
		}
	}
	return "", -1
}

func (m *Manager) IsDeviceEnabled(devPath dbus.ObjectPath) (enabled bool, busErr *dbus.Error) {
	enabled, err := m.sysNetwork.IsDeviceEnabled(0, string(devPath))
	return enabled, dbusutil.ToError(err)
}

func (m *Manager) EnableDevice(devPath dbus.ObjectPath, enabled bool) *dbus.Error {
	// device is active clicked, need auto connect ActivateConnection
	err := m.enableDevice(devPath, enabled, true)
	return dbusutil.ToError(err)
}

func (m *Manager) enableDevice(devPath dbus.ObjectPath, enabled bool, activate bool) (err error) {
	cpath, err := m.sysNetwork.EnableDevice(0, string(devPath), enabled)
	if err != nil {
		return
	}
	logger.Debugf("dev %v, enabled: %v, activate: %v", devPath, enabled, activate)
	// set enable device state
	m.setDeviceEnabled(enabled, devPath)
	// check if need activate connection
	if enabled {
		if cpath != "/" && activate {
			var uuid string
			uuid, err = nmGetConnectionUuid(cpath)
			if err != nil {
				return
			}
			_, err = m.activateConnection(uuid, devPath)
			if err != nil {
				logger.Debug("failed to activate a connection")
				return
			}
			logger.Debugf("active connection success, dev: %v, uuid: %v", devPath, uuid)
		}

		// activate connection first, then set auto-connect state,
		// in case, auto-connect is set, nm cancel block connect and try to auto-activating
		logger.Debugf("begin to set device auto-connect state, dev: %v", devPath)
		nmDev, err := nmNewDevice(devPath)
		if err != nil {
			logger.Warningf("create device failed, err: %v", err)
			return err
		}
		// set auto-connect state,
		// when device is enabled, set auto-connect as true
		// when disconnect, auto-connect is set to false automatic
		err = nmDev.Device().Autoconnect().Set(0, true)
		if err != nil {
			logger.Warningf("set device auto-connect failed, err: %v", err)
			return err
		}
		logger.Debugf("device set auto-connect success, dev: %v", devPath)
	}

	return
}

func (m *Manager) setDeviceEnabled(enabled bool, devPath dbus.ObjectPath) {
	m.stateHandler.locker.Lock()
	defer m.stateHandler.locker.Unlock()
	dsi, ok := m.stateHandler.devices[devPath]
	if !ok {
		return
	}
	dsi.enabled = enabled
	return
}

func (m *Manager) getDeviceEnabled(devPath dbus.ObjectPath) (bool, error) {
	m.stateHandler.locker.Lock()
	defer m.stateHandler.locker.Unlock()
	dsi, ok := m.stateHandler.devices[devPath]
	if !ok {
		return false, errors.New("device path not exist")
	}
	return dsi.enabled, nil
}

// SetDeviceManaged set target device managed or unmnaged from
// NetworkManager, and a little difference with other interface is
// that devPathOrIfc could be a device DBus path or the device
// interface name.
func (m *Manager) SetDeviceManaged(devPathOrIfc string, managed bool) *dbus.Error {
	err := m.setDeviceManaged(devPathOrIfc, managed)
	return dbusutil.ToError(err)
}

func (m *Manager) setDeviceManaged(devPathOrIfc string, managed bool) (err error) {
	var devPath dbus.ObjectPath
	if strings.HasPrefix(devPathOrIfc, "/org/freedesktop/NetworkManager/Devices") {
		devPath = dbus.ObjectPath(devPathOrIfc)
	} else {
		m.devicesLock.Lock()
		defer m.devicesLock.Unlock()
	out:
		for _, devs := range m.devices {
			for _, dev := range devs {
				if dev.Interface == devPathOrIfc {
					devPath = dev.Path
					break out
				}
			}
		}
	}
	if len(devPath) > 0 {
		err = nmSetDeviceManaged(devPath, managed)
	} else {
		err = fmt.Errorf("invalid device identifier: %s", devPathOrIfc)
		logger.Error(err)
	}
	return
}

// ListDeviceConnections return the available connections for the device
func (m *Manager) ListDeviceConnections(devPath dbus.ObjectPath) (connections []dbus.ObjectPath, busErr *dbus.Error) {
	connections, err := m.listDeviceConnections(devPath)
	return connections, dbusutil.ToError(err)
}

func (m *Manager) listDeviceConnections(devPath dbus.ObjectPath) ([]dbus.ObjectPath, error) {
	nmDev, err := nmNewDevice(devPath)
	if err != nil {
		return nil, err
	}

	// ignore virtual network interfaces
	if isVirtualDeviceIfc(nmDev) {
		driver, _ := nmDev.Device().Driver().Get(0)
		err = fmt.Errorf("ignore virtual network interface which driver is %s %s", driver, devPath)
		logger.Info(err)
		return nil, err
	}

	devType, _ := nmDev.Device().DeviceType().Get(0)
	if !isDeviceTypeValid(devType) {
		err = fmt.Errorf("ignore invalid device type %d", devType)
		logger.Info(err)
		return nil, err
	}

	availableConnections, _ := nmDev.Device().AvailableConnections().Get(0)
	return availableConnections, nil
}

// RequestWirelessScan request all wireless devices re-scan access point list.
func (m *Manager) RequestWirelessScan() *dbus.Error {
	m.devicesLock.Lock()
	defer m.devicesLock.Unlock()
	if devices, ok := m.devices[deviceWifi]; ok {
		for _, dev := range devices {
			err := dev.nmDev.Wireless().RequestScan(0, nil)
			if err != nil {
				logger.Debug(err)
			}
		}
	}
	return nil
}

func (m *Manager) wirelessReActiveConnection(nmDev nmdbus.Device) error {
	wireless := nmDev.Wireless()
	apPath, err := wireless.ActiveAccessPoint().Get(0)
	if err != nil {
		return err
	}
	if apPath == "/" {
		logger.Debug("Invalid active access point path:", nmDev.Path_())
		return nil
	}
	connPath, err := nmDev.Device().ActiveConnection().Get(0)
	if err != nil {
		return err
	}
	if connPath == "/" {
		logger.Debug("Invalid active connection path:", nmDev.Path_())
		return nil
	}

	connObj, err := nmNewActiveConnection(connPath)
	if err != nil {
		return err
	}

	// check network connectivity state
	state, err := connObj.State().Get(0)
	if err != nil {
		return err
	}
	if state != nm.NM_ACTIVE_CONNECTION_STATE_ACTIVATED {
		logger.Debug("[Inactive] re-active connection not activated:", connPath, nmDev.Path_(), state)
		return nil
	}

	spePath, err := connObj.SpecificObject().Get(0)
	if err != nil {
		return err
	}
	if spePath == "/" {
		logger.Debug("Invalid specific access point path:", connObj.Path_(), nmDev.Path_())
		return nil
	}

	if string(apPath) == string(spePath) {
		logger.Debug("[NONE] re-active connection not changed:", connPath, spePath, nmDev.Path_())
		return nil
	}

	ip4Path, _ := connObj.Ip4Config().Get(0)
	if m.checkGatewayConnectivity(ip4Path) {
		logger.Debug("Network is connectivity, don't re-active")
		return nil
	}

	settingsPath, err := connObj.Connection().Get(0)
	if err != nil {
		return err
	}
	logger.Debug("[DO] re-active connection:", settingsPath, connPath, spePath, nmDev.Path_())
	_, err = nmActivateConnection(settingsPath, nmDev.Path_())
	return err
}

func (m *Manager) checkGatewayConnectivity(ipPath dbus.ObjectPath) bool {
	if ipPath == "/" {
		return false
	}

	ip4Info := nmGetIp4ConfigInfo(ipPath)
	logger.Debugf("The active connection ip4 info: %v",
		ip4Info)
	// check whether the gateway is connected by ping
	if len(ip4Info.Gateway) != 0 {
		if !m.doPing(ip4Info.Gateway, 3) {
			return false
		}
	}
	return true
}

func (m *Manager) doPing(addr string, retries int) bool {
	for i := 0; i < retries; i++ {
		err := m.sysNetwork.Ping(0, addr)
		if err == nil {
			return true
		}
		logger.Warning("Failed to ping gateway:", i, addr, err)
		time.Sleep(time.Millisecond * 500)
	}
	return false
}
