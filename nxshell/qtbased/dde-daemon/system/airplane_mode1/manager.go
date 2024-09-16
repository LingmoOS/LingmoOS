// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package airplane_mode

import (
	"errors"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	networkmanager "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.networkmanager"
	polkit "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.policykit1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

const (
	dbusServiceName = "org.deepin.dde.AirplaneMode1"
	dbusPath        = "/org/deepin/dde/AirplaneMode1"
	dbusInterface   = dbusServiceName

	actionId = "org.deepin.dde.airplane-mode.enable-disable-any"
)

type device struct {
	typ  rfkillType
	soft rfkillState
	hard rfkillState
}

//go:generate dbusutil-gen -type Manager manager.go
//go:generate dbusutil-gen em -type Manager

type Manager struct {
	service         *dbusutil.Service
	btRfkillDevices map[uint32]device
	btDevicesMu     sync.RWMutex
	// Airplane Mode status
	Enabled          bool
	HasAirplaneMode  bool
	WifiEnabled      bool
	BluetoothEnabled bool

	nmManager            networkmanager.Manager
	hasNmWirelessDevices bool

	sigLoop *dbusutil.SignalLoop
	// all rfkill module config
	config *Config
}

// NewManager create manager
func newManager(service *dbusutil.Service) *Manager {
	mgr := &Manager{
		service:         service,
		btRfkillDevices: make(map[uint32]device),
		config:          NewConfig(),
	}
	err := mgr.init()
	if err != nil {
		logger.Warningf("init manager failed, err: %v", err)
	}

	return mgr
}

func (mgr *Manager) GetInterfaceName() string {
	return dbusInterface
}

func (mgr *Manager) DumpState() *dbus.Error {
	return nil
}

// Enable enable or disable *Airplane Mode*, isn't enable the devices
func (mgr *Manager) Enable(sender dbus.Sender, enableAirplaneMode bool) *dbus.Error {
	// check auth
	err := checkAuthorization(actionId, string(sender))
	if err != nil {
		logger.Warningf("checkAuthorization failed, err: %v, actionId=%v", err, actionId)
		return dbusutil.ToError(err)
	}

	// try to block
	err = mgr.block(rfkillTypeAll, enableAirplaneMode)
	if err != nil {
		logger.Warningf("block all radio failed, err: %v", err)
		return dbusutil.ToError(err)
	}

	return nil
}

// EnableWifi enable or disable *Airplane Mode* for wlan, isn't enable the wlan devices
func (mgr *Manager) EnableWifi(sender dbus.Sender, enableAirplaneMode bool) *dbus.Error {
	err := checkAuthorization(actionId, string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}

	// try to block
	err = mgr.block(rfkillTypeWifi, enableAirplaneMode)
	if err != nil {
		logger.Warningf("block wifi radio failed, err: %v", err)
		return dbusutil.ToError(err)
	}

	return nil
}

// EnableBluetooth enable or disable *Airplane Mode* for bluetooth, isn't enable the bluetooth devices
func (mgr *Manager) EnableBluetooth(sender dbus.Sender, enableAirplaneMode bool) *dbus.Error {
	err := checkAuthorization(actionId, string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}

	// try to block
	err = mgr.block(rfkillTypeBT, enableAirplaneMode)
	if err != nil {
		logger.Warningf("block bluetooth radio failed, err: %v", err)
		return dbusutil.ToError(err)
	}

	return nil
}

// init use to init manager
func (mgr *Manager) init() error {
	// load config file
	err := mgr.config.LoadConfig()
	if err != nil {
		logger.Debugf("load airplane module config failed, err: %v", err)
	}
	mgr.nmManager = networkmanager.NewManager(mgr.service.Conn())
	mgr.sigLoop = dbusutil.NewSignalLoop(mgr.service.Conn(), 10)
	mgr.sigLoop.Start()
	mgr.nmManager.InitSignalExt(mgr.sigLoop, true)
	mgr.hasNmWirelessDevices = mgr.hasWirelessDevicesWithRetry()
	mgr.initBTRfkillDevice()
	// recover
	mgr.recover()
	// use goroutine to monitor rfkill event
	go mgr.listenRfkill()
	mgr.listenWirelessEnabled()
	mgr.listenNMDevicesChanged()

	return nil
}

// recover recover origin state from config
func (mgr *Manager) recover() {
	logger.Debug("recover last state")
	// bluetooth
	mgr.BluetoothEnabled = mgr.config.GetBlocked(rfkillTypeBT)
	// bluetooth enabled, means bluetooth soft/hard block is enabled last time
	// should recover block state here
	err := mgr.block(rfkillTypeBT, mgr.BluetoothEnabled)
	if err != nil {
		logger.Warningf("recover bluetooth failed, state: %v, err: %v", mgr.BluetoothEnabled, err)
	}

	// wlan
	mgr.WifiEnabled = mgr.config.GetBlocked(rfkillTypeWifi)
	// wifi enabled, means wlan soft/hard block is enabled last time
	// should recover block state here
	err = mgr.block(rfkillTypeWifi, mgr.WifiEnabled)
	if err != nil {
		logger.Warningf("recover wifi failed, state: %v, err: %v", mgr.WifiEnabled, err)
	}

	// all
	mgr.Enabled = mgr.config.GetBlocked(rfkillTypeAll)
	// enabled, means all soft/hard block is enabled last time
	// should recover block state here
	err = mgr.block(rfkillTypeAll, mgr.Enabled)
	if err != nil {
		logger.Warningf("recover all failed, state: %v, err: %v", mgr.Enabled, err)
	}

	mgr.btDevicesMu.RLock()
	defer mgr.btDevicesMu.RUnlock()
	mgr.setPropHasAirplaneMode(len(mgr.btRfkillDevices) > 0 || mgr.hasNmWirelessDevices)
}

// block use rfkill to block wifi
func (mgr *Manager) block(typ rfkillType, enableAirplaneMode bool) error {
	state := rfkillStateUnblock
	if enableAirplaneMode {
		state = rfkillStateBlock
	}
	if typ < 2 { // 无线网卡飞行模式通过NM管控
		if mgr.hasNmWirelessDevices {
			err := mgr.nmManager.WirelessEnabled().Set(0, !enableAirplaneMode)
			if err != nil {
				logger.Warning(err)
			}
		}
	}
	return rfkillAction(typ, state)
}

func checkAuthorization(actionId string, sysBusName string) error {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	authority := polkit.NewAuthority(systemBus)
	subject := polkit.MakeSubject(polkit.SubjectKindSystemBusName)
	subject.SetDetail("name", sysBusName)

	ret, err := authority.CheckAuthorization(0, subject, actionId,
		nil, polkit.CheckAuthorizationFlagsAllowUserInteraction, "")
	if err != nil {
		return err
	}
	if !ret.IsAuthorized {
		return errors.New("not authorized")
	}
	return nil
}

func (mgr *Manager) listenWirelessEnabled() {
	_ = mgr.nmManager.WirelessEnabled().ConnectChanged(func(hasValue bool, wifiEnable bool) {
		if !hasValue {
			return
		}
		// 硬件关闭时，开启wifi会先收到enable变为true 然后变为false
		hardEnable, err := mgr.nmManager.WirelessHardwareEnabled().Get(0)
		if err != nil {
			logger.Warning(err)
		}
		wifiAirplaneMode := !wifiEnable || !hardEnable
		mgr.setPropWifiEnabled(wifiAirplaneMode)
		mgr.config.SetBlocked(rfkillTypeWifi, !wifiEnable) // 无法判断wifi是否为soft block
		mgr.btDevicesMu.RLock()
		defer mgr.btDevicesMu.RUnlock()

		mgr.updateAllState()
		err = mgr.config.SaveConfig()
		if err != nil {
			logger.Warningf("save rfkill config file failed, err: %v", err)
		}
		logger.Debugf("rfkill state, bluetooth: %v, wifi: %v, airplane: %v", mgr.BluetoothEnabled, mgr.WifiEnabled, mgr.Enabled)
	})
}

func (mgr *Manager) listenNMDevicesChanged() {
	_ = mgr.nmManager.Devices().ConnectChanged(func(hasValue bool, value []dbus.ObjectPath) {
		if !hasValue {
			return
		}
		mgr.hasNmWirelessDevices = mgr.hasWirelessDevices(value)
		mgr.btDevicesMu.RLock()
		defer mgr.btDevicesMu.RUnlock()
		mgr.setPropHasAirplaneMode(len(mgr.btRfkillDevices) > 0 || mgr.hasNmWirelessDevices)
	})
}

func (mgr *Manager) hasWirelessDevicesWithRetry() bool {
	// try get all devices 5 times
	for i := 0; i < 5; i++ {
		devicePaths, err := mgr.nmManager.GetDevices(0)
		if err != nil {
			logger.Warning(err)
			// sleep for 1 seconds, and retry get devices
			time.Sleep(1 * time.Second)
		} else {
			return mgr.hasWirelessDevices(devicePaths)
		}
	}
	return false
}

const NM_DEVICE_TYPE_WIFI = 2

// add and check devices
func (mgr *Manager) hasWirelessDevices(devicePaths []dbus.ObjectPath) bool {
	for _, devPath := range devicePaths {
		nmDev, err := networkmanager.NewDevice(mgr.service.Conn(), devPath)
		if err != nil {
			logger.Warning(err)
			continue
		}
		d := nmDev.Device()
		deviceType, err := d.DeviceType().Get(0)
		if err != nil {
			logger.Warningf("get device %s type failed: %v", nmDev.Path_(), err)
		}
		if deviceType == NM_DEVICE_TYPE_WIFI {
			return true
		}
	}
	return false
}
