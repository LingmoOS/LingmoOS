// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"fmt"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	ConfigManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	bluez "github.com/linuxdeepin/go-dbus-factory/system/org.bluez"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

const (
	bluezDBusServiceName      = "org.bluez"
	bluezAdapterDBusInterface = "org.bluez.Adapter1"
	bluezDeviceDBusInterface  = "org.bluez.Device1"
	bluezBatteryDBusInterface = "org.bluez.Battery1"

	dbusServiceName = "org.deepin.dde.Bluetooth1"
	dbusPath        = "/org/deepin/dde/Bluetooth1"
	dbusInterface   = dbusServiceName
)

const (
	StateUnavailable = 0
	StateAvailable   = 1
	StateConnected   = 2
)

const (
	devIconComputer        = "computer"
	devIconPhone           = "phone"
	devIconModem           = "modem"
	devIconNetworkWireless = "network-wireless"
	devIconAudioCard       = "audio-card"
	devIconCameraVideo     = "camera-video"
	devIconCameraPhoto     = "camera-photo"
	devIconPrinter         = "printer"
	devIconInputGaming     = "input-gaming"
	devIconInputKeyboard   = "input-keyboard"
	devIconInputTablet     = "input-tablet"
	devIconInputMouse      = "input-mouse"
)

const (
	dsettingsAppID               = "org.deepin.dde.daemon"
	dsettingsBluetoothName       = "org.deepin.dde.daemon.bluetooth"
	dsettingsAutoPairEnableKey   = "autoPairEnable"
	dsettingsAdapterPowerd       = "adapterPowerd"
	dsettingsAdapterDiscoverable = "adapterDiscoverable"
)

//go:generate dbusutil-gen -type SysBluetooth bluetooth.go
//go:generate dbusutil-gen em -type SysBluetooth,agent

type SysBluetooth struct {
	service       *dbusutil.Service
	sigLoop       *dbusutil.SignalLoop
	config        *config
	objectManager bluez.ObjectManager
	sysDBusDaemon ofdbus.DBus
	loginManager  login1.Manager
	agent         *agent
	userAgents    *userAgentMap

	// adapter
	adaptersMu sync.Mutex
	adapters   map[dbus.ObjectPath]*adapter

	// device
	devicesMu sync.Mutex
	devices   map[dbus.ObjectPath][]*device

	// backup device
	backupDevicesMu sync.Mutex
	backupDevices   map[dbus.ObjectPath][]*backupDevice

	acm *autoConnectManager

	PropsMu     sync.RWMutex
	State       uint32 // StateUnavailable/StateAvailable/StateConnected
	CanSendFile bool

	// 当发起设备连接成功后，应该把连接的设备添加进设备列表
	connectedDevices map[dbus.ObjectPath][]*device
	connectedMu      sync.RWMutex
	// 设备被清空后需要连接的设备路径
	prepareToConnectedDevice dbus.ObjectPath
	prepareToConnectedMu     sync.Mutex
	// 升级后第一次进系统，此时需要根据之前有蓝牙连接时，打开蓝牙开关
	needFixBtPoweredStatus bool
	canAutoPair            func() bool

	// nolint
	signals *struct {
		// adapter/device properties changed signals
		AdapterAdded, AdapterRemoved, AdapterPropertiesChanged struct {
			adapterJSON string
		}

		DeviceAdded, DeviceRemoved, DevicePropertiesChanged struct {
			devJSON string
		}
	}
}

func newSysBluetooth(service *dbusutil.Service) (b *SysBluetooth) {
	sysBus := service.Conn()
	b = &SysBluetooth{
		service:                service,
		sigLoop:                dbusutil.NewSignalLoop(sysBus, 100),
		userAgents:             newUserAgentMap(),
		needFixBtPoweredStatus: false,
	}

	b.config = newConfig()
	b.loginManager = login1.NewManager(sysBus)
	b.sysDBusDaemon = ofdbus.NewDBus(sysBus)
	b.objectManager = bluez.NewObjectManager(sysBus)
	b.adapters = make(map[dbus.ObjectPath]*adapter)
	b.devices = make(map[dbus.ObjectPath][]*device)
	b.backupDevices = make(map[dbus.ObjectPath][]*backupDevice)
	b.connectedDevices = make(map[dbus.ObjectPath][]*device)
	b.acm = newAutoConnectManager()
	b.acm.connectCb = func(adapterPath, devicePath dbus.ObjectPath, wId int) error {
		adapter, err := b.getAdapter(adapterPath)
		if err != nil {
			// 可能是适配器被移除了，停止连接
			return nil
		}
		if !adapter.Powered {
			// 适配器电源关闭了，停止连接
			return nil
		}

		adapter.autoConnectFinished = false
		return b.autoConnectPairedDevice(devicePath, adapterPath)
	}

	b.acm.startDiscoveryCb = func(adapterPath dbus.ObjectPath) {
		adapter, err := b.getAdapter(adapterPath)
		if err != nil {
			// 可能是适配器被移除了，不需要开始扫描
			logger.Warningf("call getAdapter failed; adapterPath:[%s] err:[%s]", adapterPath, err)
			return
		}

		if !adapter.Powered {
			// 适配器电源关闭了，不需要开始扫描
			logger.Warningf("adapter： [%s] is power off", adapterPath)
			return
		}

		adapter.autoConnectFinished = true
		adapter.startDiscovery()
	}

	return
}

func (b *SysBluetooth) destroy() {
	b.agent.destroy()

	b.objectManager.RemoveAllHandlers()
	b.sysDBusDaemon.RemoveAllHandlers()
	b.loginManager.RemoveAllHandlers()

	b.devicesMu.Lock()
	for _, devices := range b.devices {
		for _, device := range devices {
			device.destroy()
		}
	}
	b.devicesMu.Unlock()

	b.adaptersMu.Lock()
	for _, adapter := range b.adapters {
		adapter.destroy()
	}
	b.adaptersMu.Unlock()

	err := b.service.StopExport(b)
	if err != nil {
		logger.Warning(err)
	}
	b.sigLoop.Stop()
}

func (*SysBluetooth) GetInterfaceName() string {
	return dbusInterface
}

func (b *SysBluetooth) init() {
	var err error
	b.CanSendFile, err = canSendFile()
	if err != nil {
		logger.Warning("canSendFile err:", err)
	}

	// 需要在Load加载之前判断系统级蓝牙配置文件是否存在
	b.needFixBtPoweredStatus = !b.config.core.IsConfigFileExists()
	b.sigLoop.Start()
	b.config.load()
	b.sysDBusDaemon.InitSignalExt(b.sigLoop, true)
	_, err = b.sysDBusDaemon.ConnectNameOwnerChanged(b.handleDBusNameOwnerChanged)
	if err != nil {
		logger.Warning(err)
	}

	ds := ConfigManager.NewConfigManager(b.sigLoop.Conn())

	dsBluetoothPath, err := ds.AcquireManager(0, dsettingsAppID, dsettingsBluetoothName, "")
	if err != nil {
		logger.Warning(err)
		return
	}

	dsBluetooth, err := ConfigManager.NewManager(b.sigLoop.Conn(), dsBluetoothPath)
	if err != nil {
		logger.Warning(err)
		return
	}

	b.canAutoPair = func() bool {
		v, err := dsBluetooth.Value(0, dsettingsAutoPairEnableKey)
		if err != nil {
			logger.Warning(err)
			return true
		}
		return v.Value().(bool)
	}

	logger.Info("====== dsg autoPairEnable is ======", b.canAutoPair())

	adapterDefaultPowered = func() bool {
		v, err := dsBluetooth.Value(0, dsettingsAdapterPowerd)
		if err != nil {
			logger.Warning(err)
			return false
		}
		return v.Value().(bool)
	}()

	adapterDefaultDiscoverable = func() bool {
		v, err := dsBluetooth.Value(0, dsettingsAdapterDiscoverable)
		if err != nil {
			logger.Warning(err)
			return true
		}
		return v.Value().(bool)
	}()

	b.loginManager.InitSignalExt(b.sigLoop, true)
	_, err = b.loginManager.ConnectSessionNew(b.handleSessionNew)

	_, err = b.loginManager.ConnectSessionRemoved(func(sessionId string, sessionPath dbus.ObjectPath) {
		logger.Info("session removed", sessionId, sessionPath)
		b.userAgents.removeSession(sessionPath)
	})
	if err != nil {
		logger.Warning(err)
	}

	_, err = b.loginManager.ConnectUserRemoved(func(uid uint32, userPath dbus.ObjectPath) {
		uidStr := strconv.Itoa(int(uid))
		b.userAgents.removeUser(uidStr)
	})

	_, err = b.loginManager.ConnectPrepareForSleep(b.handlePrepareForSleep)
	if err != nil {
		logger.Warning(err)
	}

	b.objectManager.InitSignalExt(b.sigLoop, true)
	_, err = b.objectManager.ConnectInterfacesAdded(b.handleInterfacesAdded)
	if err != nil {
		logger.Warning(err)
	}

	_, err = b.objectManager.ConnectInterfacesRemoved(b.handleInterfacesRemoved)
	if err != nil {
		logger.Warning(err)
	}

	b.agent.init()
	b.loadObjects()

	b.config.clearSpareConfig(b)
	go b.tryConnectPairedDevices("")
}

const btAvailableFile = "/sys/kernel/security/bluetooth/available"

func canSendFile() (can bool, err error) {
	_, err = os.Stat(btAvailableFile)
	if err != nil {
		if os.IsNotExist(err) {
			// 表示没有加载限制性的内核模块
			return true, nil
		}
		return false, err
	}

	can, err = readBoolFile(btAvailableFile)
	return
}

func (b *SysBluetooth) handleSessionNew(sessionId string, sessionPath dbus.ObjectPath) {
	logger.Info("session added", sessionId, sessionPath)
	sysBus := b.sigLoop.Conn()
	session, err := login1.NewSession(sysBus, sessionPath)
	if err != nil {
		logger.Warning(err)
		return
	}

	userInfo, err := session.User().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}

	uidStr := strconv.Itoa(int(userInfo.UID))
	if !b.userAgents.hasUser(uidStr) {
		// 不关心这个用户的新 session
		return
	}

	newlyAdded := b.userAgents.addSession(uidStr, session)
	if newlyAdded {
		b.watchSession(uidStr, session)
	}
}

func (b *SysBluetooth) handlePrepareForSleep(beforeSleep bool) {
	if beforeSleep {
		b.stopDiscovery()
		logger.Debug("prepare to sleep")
		return
	}
	logger.Debug("Wakeup from sleep, will set adapter and try connect device")
	time.AfterFunc(time.Second*3, func() {
		// for _, adapter := range b.adapters {
		//	if !adapter.Powered {
		//		continue
		//	}
		//	//_ = adapter.core.Adapter().Discoverable().Set(0, b.config.Discoverable)
		// }
		b.tryConnectPairedDevices("")
	})
}

func (b *SysBluetooth) watchSession(uid string, session login1.Session) {
	session.InitSignalExt(b.sigLoop, true)
	err := session.Active().ConnectChanged(func(hasValue bool, active bool) {
		if !hasValue {
			return
		}
		if active {
			b.userAgents.setActiveUid(uid)
		}
	})

	if err != nil {
		logger.Warning(err)
	}

	active, err := session.Active().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	if active {
		b.userAgents.setActiveUid(uid)
	}
}

func (b *SysBluetooth) loadObjects() {
	// add exists adapters and devices
	objects, err := b.objectManager.GetManagedObjects(0)
	if err != nil {
		logger.Error(err)
		return
	}

	// add adapters
	for path, obj := range objects {
		if _, ok := obj[bluezAdapterDBusInterface]; ok {
			b.addAdapter(path)
		}
	}

	// then add devices
	for path, obj := range objects {
		if _, ok := obj[bluezDeviceDBusInterface]; ok {
			b.addDevice(path)
		}
	}

	// then update battery
	for path, obj := range objects {
		if _, ok := obj[bluezBatteryDBusInterface]; ok {
			b.updateBatteryForAdd(path)
		}
	}
}

func (b *SysBluetooth) removeAllObjects() {
	b.devicesMu.Lock()
	for _, devices := range b.devices {
		for _, device := range devices {
			device.notifyDeviceRemoved()
			device.destroy()
		}
	}
	b.devices = make(map[dbus.ObjectPath][]*device)
	b.devicesMu.Unlock()

	b.adaptersMu.Lock()
	for _, adapter := range b.adapters {
		adapter.notifyAdapterRemoved()
		adapter.destroy()
	}
	b.adapters = make(map[dbus.ObjectPath]*adapter)
	b.adaptersMu.Unlock()
}

func (b *SysBluetooth) handleInterfacesAdded(path dbus.ObjectPath, data map[string]map[string]dbus.Variant) {
	if _, ok := data[bluezAdapterDBusInterface]; ok {
		b.addAdapter(path)
	}
	if _, ok := data[bluezDeviceDBusInterface]; ok {
		b.addDeviceWithCount(path, 3)
	}
	if _, ok := data[bluezBatteryDBusInterface]; ok {
		b.updateBatteryForAdd(path)
	}
}

func (b *SysBluetooth) handleInterfacesRemoved(path dbus.ObjectPath, interfaces []string) {
	if isStringInArray(bluezAdapterDBusInterface, interfaces) {
		b.removeAdapter(path)
	}
	if isStringInArray(bluezDeviceDBusInterface, interfaces) {
		b.removeDevice(path)
	}
	if isStringInArray(bluezBatteryDBusInterface, interfaces) {
		b.updateBatteryForRemove(path)
	}
}

func (b *SysBluetooth) handleDBusNameOwnerChanged(name, oldOwner, newOwner string) {
	// if a new dbus session was installed, the name and newOwner
	// will be not empty, if a dbus session was uninstalled, the
	// name and oldOwner will be not empty
	if name == bluezDBusServiceName {
		if newOwner != "" {
			logger.Info("bluetooth is starting")
			time.AfterFunc(1*time.Second, func() {
				b.loadObjects()
				b.agent.registerDefaultAgent()
				b.tryConnectPairedDevices("")
			})
		} else {
			logger.Info("bluetooth stopped")
			b.removeAllObjects()
		}
	} else {
		if strings.HasPrefix(name, ":") && oldOwner != "" && newOwner == "" {
			b.userAgents.handleNameLost(name)
		}
	}
}

func (b *SysBluetooth) addDeviceWithCount(devPath dbus.ObjectPath, count int) {
	logger.Debug("receive signal device added", devPath)
	if b.isDeviceExists(devPath) {
		return
	}

	d := newDevice(b.sigLoop, devPath)
	b.adaptersMu.Lock()
	d.adapter = b.adapters[d.AdapterPath]
	b.adaptersMu.Unlock()

	if d.adapter == nil {
		logger.Warningf("failed to add device %s, not found adapter", devPath)
		if count > 0 {
			logger.Debugf("retry add device %s after 100ms", devPath)
			time.AfterFunc(100*time.Millisecond, func() {
				b.addDeviceWithCount(devPath, count-1)
			})
		}
		return
	}

	// device detail info is needed to write into config file
	b.config.addDeviceConfig(d)

	b.devicesMu.Lock()
	b.devices[d.AdapterPath] = append(b.devices[d.AdapterPath], d)
	b.devicesMu.Unlock()

	// 设备加入的同时进行备份
	b.backupDevicesMu.Lock()
	idx := b.indexBackupDeviceNoLock(d.AdapterPath, devPath)
	if idx == -1 {
		b.backupDevices[d.AdapterPath] = append(b.backupDevices[d.AdapterPath], newBackupDevice(d))
	}
	b.backupDevicesMu.Unlock()

	d.notifyDeviceAdded()

	// 若扫描到需要连接的设备，直接连接
	if b.prepareToConnectedDevice == d.Path {
		b.prepareToConnectedDevice = ""
		go func() {
			err := d.Connect()
			if err != nil {
				logger.Warning(err)
			}
		}()
	}
}

func (b *SysBluetooth) updateBatteryForAdd(devPath dbus.ObjectPath) {
	if !b.isDeviceExists(devPath) {
		b.addDeviceWithCount(devPath, 3)
	} else {
		d, _ := b.getDevice(devPath)
		d.Battery, _ = d.core.Percentage().Get(0)

		// update backup battery
		b.backupDevicesMu.Lock()
		idx := b.indexBackupDeviceNoLock(d.AdapterPath, devPath)
		if idx != -1 {
			b.backupDevices[d.AdapterPath][idx].Battery = d.Battery
		} else {
			b.backupDevices[d.AdapterPath] = append(b.backupDevices[d.AdapterPath], newBackupDevice(d))
		}
		b.backupDevicesMu.Unlock()

		// notify prop changed
		d.notifyDevicePropertiesChanged()
	}
}

func (b *SysBluetooth) updateBatteryForRemove(devPath dbus.ObjectPath) {
	if b.isDeviceExists(devPath) {
		d, _ := b.getDevice(devPath)
		d.Battery = 0

		// update backup battery
		b.backupDevicesMu.Lock()
		idx := b.indexBackupDeviceNoLock(d.AdapterPath, devPath)
		if idx != -1 {
			b.backupDevices[d.AdapterPath][idx].Battery = 0
		}
		b.backupDevicesMu.Unlock()

		// notify prop changed
		d.notifyDevicePropertiesChanged()
	}
}

func (b *SysBluetooth) addDevice(devPath dbus.ObjectPath) {
	b.addDeviceWithCount(devPath, 0)
}

func (b *SysBluetooth) removeDevice(devPath dbus.ObjectPath) {
	logger.Debug("receive signal device removed", devPath)
	adapterPath, idx := b.getDeviceIndex(devPath)
	if idx == -1 {
		logger.Warning("repeat remove device", devPath)
		return
	}

	var removedDev *device
	b.devicesMu.Lock()
	b.devices[adapterPath], removedDev = b.doRemoveDevice(b.devices[adapterPath], idx)
	b.devicesMu.Unlock()

	b.acm.handleDeviceEvent(removedDev)

	idx = b.indexBackupDevice(adapterPath, devPath)
	if idx == -1 {
		// 未找到备份设备，需要发送设备移除信号
		removedDev.notifyDeviceRemoved()
	} else {
		// 找得到备份设备，但是是已经配对的设备，一般是通过 bluetoothctl remove 命令删除已经配对的设备。
		// 备份设备也删除，并发送信号。
		adapter := b.adapters[adapterPath]
		if removedDev.Paired || (adapter != nil && adapter.Discovering) {
			b.removeBackupDevice(devPath)
			removedDev.notifyDeviceRemoved()
		}
	}
}

func (b *SysBluetooth) doRemoveDevice(devices []*device, i int) ([]*device, *device) {
	// NOTE: do not remove device from config
	d := devices[i]
	d.destroy()
	copy(devices[i:], devices[i+1:])
	devices[len(devices)-1] = nil
	devices = devices[:len(devices)-1]
	return devices, d
}

func (b *SysBluetooth) removeBackupDevice(devPath dbus.ObjectPath) {
	b.backupDevicesMu.Lock()
	defer b.backupDevicesMu.Unlock()

	adapterPath, i := b.findBackupDeviceNoLock(devPath)
	if i < 0 {
		logger.Debug("repeat remove device", devPath)
		return
	}

	b.backupDevices[adapterPath] = b.doRemoveBackupDevice(b.backupDevices[adapterPath], i)
}

func (b *SysBluetooth) doRemoveBackupDevice(devices []*backupDevice, i int) []*backupDevice {
	copy(devices[i:], devices[i+1:])
	devices[len(devices)-1] = nil
	devices = devices[:len(devices)-1]
	return devices
}

func (b *SysBluetooth) syncCommonToBackupDevices(adapterPath dbus.ObjectPath) {
	logger.Debug("syncCommonToBackupDevices", adapterPath)
	devices := b.getDevices(adapterPath)

	// 转换为 map
	deviceMap := make(map[dbus.ObjectPath]*device)
	for _, d := range devices {
		deviceMap[d.Path] = d
	}

	// 应该只需要关注备份的设备比普通设备多的情况。
	var removedBackupDevices []*backupDevice
	var newBackupDevices []*backupDevice

	b.backupDevicesMu.Lock()
	for _, backupDevice := range b.backupDevices[adapterPath] {
		// 从 devices 能找到，保留，不能找到，则删除
		_, ok := deviceMap[backupDevice.Path]
		if !ok {
			removedBackupDevices = append(removedBackupDevices, backupDevice)
		}
	}

	for _, d := range devices {
		newBackupDevices = append(newBackupDevices, newBackupDevice(d))
	}
	b.backupDevices[adapterPath] = newBackupDevices
	b.backupDevicesMu.Unlock()

	for _, d := range removedBackupDevices {
		d.notifyDeviceRemoved()
	}
}

func (b *SysBluetooth) isDeviceExists(devPath dbus.ObjectPath) bool {
	_, i := b.getDeviceIndex(devPath)
	return i >= 0
}

func (b *SysBluetooth) findDevice(devPath dbus.ObjectPath) (adapterPath dbus.ObjectPath, index int) {
	for p, devices := range b.devices {
		for i, d := range devices {
			if d.Path == devPath {
				return p, i
			}
		}
	}
	return "", -1
}

func (b *SysBluetooth) findBackupDevice(devPath dbus.ObjectPath) (adapterPath dbus.ObjectPath, index int) {
	b.backupDevicesMu.Lock()
	defer b.backupDevicesMu.Unlock()
	return b.findBackupDeviceNoLock(devPath)
}

func (b *SysBluetooth) findBackupDeviceNoLock(devPath dbus.ObjectPath) (adapterPath dbus.ObjectPath, index int) {
	for p, devices := range b.backupDevices {
		for i, d := range devices {
			if d.Path == devPath {
				return p, i
			}
		}
	}
	return "", -1
}

func (b *SysBluetooth) indexBackupDevice(adapterPath, devPath dbus.ObjectPath) (index int) {
	b.backupDevicesMu.Lock()
	defer b.backupDevicesMu.Unlock()
	return b.indexBackupDeviceNoLock(adapterPath, devPath)
}

func (b *SysBluetooth) indexBackupDeviceNoLock(adapterPath, devPath dbus.ObjectPath) (index int) {
	devices, ok := b.backupDevices[adapterPath]
	if !ok {
		return -1
	}
	for idx, d := range devices {
		if d.Path == devPath {
			return idx
		}
	}
	return -1
}

func (b *SysBluetooth) getDeviceIndex(devPath dbus.ObjectPath) (adapterPath dbus.ObjectPath, index int) {
	b.devicesMu.Lock()
	defer b.devicesMu.Unlock()
	return b.findDevice(devPath)
}

func (b *SysBluetooth) getDevice(devPath dbus.ObjectPath) (*device, error) {
	b.devicesMu.Lock()
	defer b.devicesMu.Unlock()
	adapterPath, index := b.findDevice(devPath)
	if index < 0 {
		return nil, errInvalidDevicePath
	}
	return b.devices[adapterPath][index], nil
}

func (b *SysBluetooth) getBackupDevice(devPath dbus.ObjectPath) (*backupDevice, error) {
	b.backupDevicesMu.Lock()
	defer b.backupDevicesMu.Unlock()
	adapterPath, index := b.findBackupDeviceNoLock(devPath)
	if index < 0 {
		return nil, errInvalidDevicePath
	}
	return b.backupDevices[adapterPath][index], nil
}

// 更新 backupDevices
func (b *SysBluetooth) updateBackupDevices(adapterPath dbus.ObjectPath) {
	logger.Debug("updateBackupDevices", adapterPath)
	devices := b.getDevices(adapterPath)
	b.backupDevicesMu.Lock()
	defer b.backupDevicesMu.Unlock()
	for _, device := range devices {
		index := b.indexBackupDeviceNoLock(adapterPath, device.Path)
		if index < 0 {
			logger.Warning("[updateBackupDevices] invalid device path: ", device.Path)
		} else {
			b.backupDevices[adapterPath][index] = newBackupDevice(device)
		}
	}
}

func (b *SysBluetooth) getAdapterDevices(adapterAddress string) []*device {
	var aPath dbus.ObjectPath
	b.adaptersMu.Lock()
	for adapterPath, adapter := range b.adapters {
		if adapter.Address == adapterAddress {
			aPath = adapterPath
			break
		}
	}
	b.adaptersMu.Unlock()

	if aPath == "" {
		return nil
	}

	b.devicesMu.Lock()
	defer b.devicesMu.Unlock()

	devices := b.devices[aPath]
	if devices == nil {
		return nil
	}

	result := make([]*device, 0, len(devices))
	result = append(result, devices...)
	return result
}

func (b *SysBluetooth) addAdapter(adapterPath dbus.ObjectPath) {
	logger.Debug("receive signal adapter added", adapterPath)
	if b.isAdapterExists(adapterPath) {
		return
	}

	a := newAdapter(b.sigLoop, adapterPath)
	a.bt = b
	// initialize adapter power state
	b.config.addAdapterConfig(a.Address)
	cfgPowered := b.config.getAdapterConfigPowered(a.Address)

	err := a.core.Adapter().Powered().Set(0, cfgPowered)
	if err != nil {
		logger.Warning(err)
	}

	err = a.core.Adapter().DiscoverableTimeout().Set(0, 0)
	if err != nil {
		logger.Warning(err)
	}

	// 读取配置文件中可被发现状态值，并修改Discoverable属性值，默认为true
	discoverable := b.config.getAdapterConfigDiscoverable(a.Address)
	err = a.core.Adapter().Discoverable().Set(0, discoverable)
	if err != nil {
		logger.Warning(err)
	}

	b.adaptersMu.Lock()
	b.adapters[adapterPath] = a
	b.adaptersMu.Unlock()

	a.notifyAdapterAdded()
	b.acm.addAdapter(adapterPath)
	logger.Debug("addAdapter", adapterPath)
}

func (b *SysBluetooth) removeAdapter(adapterPath dbus.ObjectPath) {
	logger.Debug("receive signal adapter removed", adapterPath)
	b.adaptersMu.Lock()

	if b.adapters[adapterPath] == nil {
		b.adaptersMu.Unlock()
		logger.Warning("repeat remove adapter", adapterPath)
		return
	}

	b.doRemoveAdapter(adapterPath)
	b.adaptersMu.Unlock()

	b.devicesMu.Lock()
	b.devices[adapterPath] = nil
	b.devicesMu.Unlock()
	b.syncCommonToBackupDevices(adapterPath)
}

func (b *SysBluetooth) doRemoveAdapter(adapterPath dbus.ObjectPath) {
	// NOTE: do not remove adapter from config file
	removeAdapter := b.adapters[adapterPath]
	delete(b.adapters, adapterPath)
	removeAdapter.notifyAdapterRemoved()
	removeAdapter.destroy()
	b.acm.removeAdapter(adapterPath)
}

func (b *SysBluetooth) getAdapter(adapterPath dbus.ObjectPath) (adapter *adapter, err error) {
	b.adaptersMu.Lock()
	defer b.adaptersMu.Unlock()

	adapter = b.adapters[adapterPath]
	if adapter == nil {
		err = fmt.Errorf("adapter not exists %s", adapterPath)
		logger.Error(err)
		return
	}
	return
}

func (b *SysBluetooth) isAdapterExists(adapterPath dbus.ObjectPath) bool {
	b.adaptersMu.Lock()
	defer b.adaptersMu.Unlock()
	return b.adapters[adapterPath] != nil
}

func (b *SysBluetooth) updateState() {
	newState := StateUnavailable
	if len(b.adapters) > 0 {
		newState = StateAvailable
	}

	for _, devices := range b.devices {
		for _, d := range devices {
			if d.adapter != nil && d.adapter.Powered && d.connected && d.Paired {
				newState = StateConnected
				break
			}
		}
	}

	b.PropsMu.Lock()
	b.setPropState(uint32(newState))
	b.PropsMu.Unlock()
}

func (b *SysBluetooth) getAdapters() []*adapter {
	b.adaptersMu.Lock()
	defer b.adaptersMu.Unlock()

	result := make([]*adapter, 0, len(b.adapters))
	for _, adapter := range b.adapters {
		result = append(result, adapter)
	}
	return result
}

func (b *SysBluetooth) getDevices(adapterPath dbus.ObjectPath) []*device {
	b.devicesMu.Lock()
	defer b.devicesMu.Unlock()

	devices := b.devices[adapterPath]
	result := make([]*device, len(devices))
	copy(result, devices)
	return result
}

func (b *SysBluetooth) tryConnectPairedDevices(adapterPath dbus.ObjectPath) {
	if b.canAutoPair != nil {
		if !b.canAutoPair() {
			logger.Info("can't auto pair beacuse autoPairEnable key is unable")
			return
		}
	} else {
		return
	}

	b.setAutoConnectFinishedStatus(adapterPath, false)

	inputOnly := true
	// 自动连接时长
	defaultConnectDuration := 2 * time.Minute
	if b.getActiveUserAgent() != nil {
		// 表示用户已经登录
		inputOnly = false
		// 可能有用户控制，为便于用户控制，缩短自动连接时长。
		defaultConnectDuration = 20 * time.Second
	}
	logger.Debugf("tryConnectPairedDevices adapterPath: %q, inputOnly: %v", adapterPath, inputOnly)
	adapterDevicesMap := b.getPairedDevicesForAutoConnect(adapterPath)

	for adapterPath, devices := range adapterDevicesMap {
		if inputOnly {
			// 登录界面，只需要输入设备
			devices = filterOutDevices(devices, func(device *device) bool {
				return strings.HasPrefix(device.Icon, "input")
			})
		}
		logger.Debug("before soft devices:", devices)
		b.config.softDevices(devices)
		logger.Debug("after soft devices:", devices)
		var deviceInfos []autoDeviceInfo
		priority := 0
		// 可能由设备主动连接的设备
		var activeReconnectDevices []*device
		for _, d := range devices {
			if d.maybeReconnectByDevice() {
				activeReconnectDevices = append(activeReconnectDevices, d)
			}
			connectDuration := defaultConnectDuration
			if d.shouldReconnectByHost() {
				logger.Debug("try auto connect", d)
			} else {
				logger.Debugf("do not auto connect %v, but try connect once", d)
				connectDuration = 1 * time.Second
				if !d.Trusted {
					err := d.core.Trusted().Set(0, true)
					if err != nil {
						logger.Warning(err)
					}
				}
				err := d.cancelBlock()
				if err != nil {
					logger.Warning(err)
				}
			}
			deviceInfos = append(deviceInfos, autoDeviceInfo{
				adapter:            adapterPath,
				device:             d.Path,
				alias:              d.Alias,
				priority:           priority,
				connectDurationMax: connectDuration,
			})
			priority++
		}
		b.setAutoConnectFinishedStatus(adapterPath, false)
		b.acm.addDevices(adapterPath, deviceInfos, activeReconnectDevices)
	}
}

func (b *SysBluetooth) autoConnectPairedDevice(devPath dbus.ObjectPath, adapterPath dbus.ObjectPath) error {
	device, err := b.getDevice(devPath)
	if err != nil {
		return err
	}

	if device.connected {
		logger.Debugf("%v is already connected", device)
		return nil
	}

	switch device.Icon {
	// 只自动连接一个这些图标的设备
	case devIconAudioCard, devIconInputKeyboard, devIconInputMouse, devIconInputTablet:
		connectedDevice := b.findFirstConnectedDeviceByIcon(device.Icon)
		if connectedDevice != nil {
			logger.Debugf("there is already a connected %v, icon: %v, do not auto connect it: %v",
				connectedDevice, device.Icon, device)
			return nil
		}
	}

	logger.Debug("auto connect paired", device)
	err = device.doConnect(false)
	if err != nil {
		logger.Debugf("failed to auto connect %v: %v", device, err)
		// 设置 connect phase 可以造成界面上一直在连接，而不中断的情况。
		device.setConnectPhase(connectPhaseConnectFailedSleep)
		time.Sleep(3 * time.Second)
		device.setConnectPhase(connectPhaseNone)
	}
	return err
}

// 当 adapterPath 为空时，获取所有适配器的配对，但未连接的设备。
// 当 adapterPath 不为空时，或者获取指定适配器的配对，但未连接的设备。
func (b *SysBluetooth) getPairedDevicesForAutoConnect(adapterPath dbus.ObjectPath) map[dbus.ObjectPath][]*device {
	adapters := b.getAdapters()
	if adapterPath != "" {
		var theAdapter *adapter
		for _, adapter := range adapters {
			if adapter.Path == adapterPath {
				theAdapter = adapter
				break
			}
		}
		if theAdapter == nil {
			return nil
		}
		adapters = []*adapter{theAdapter}
	}
	result := make(map[dbus.ObjectPath][]*device)
	for _, adapter := range adapters {
		// 当适配器打开电源时才自动连接
		if !adapter.Powered {
			continue
		}
		devices := b.getDevices(adapter.Path)
		var tmpDevices []*device
		for _, d := range devices {
			if d != nil && d.Paired && !d.connected {
				tmpDevices = append(tmpDevices, d)
			}
		}

		result[adapter.Path] = tmpDevices
	}

	return result
}

// 判断设备是否为经典蓝牙设备
func (d *device) isBREDRDevice() bool {
	technologies, err := d.getTechnologies()
	if err != nil {
		logger.Warningf("failed to get %v technologies: %v",
			d, err)
		return false
	}
	for _, tech := range technologies {
		if tech == "BR/EDR" {
			return true
		}
	}
	return false
}

func (b *SysBluetooth) addConnectedDevice(connectedDev *device) {
	b.connectedMu.Lock()
	b.connectedDevices[connectedDev.AdapterPath] = append(b.connectedDevices[connectedDev.AdapterPath], connectedDev)
	b.connectedMu.Unlock()
}

func (b *SysBluetooth) removeConnectedDevice(disconnectedDev *device) {
	b.connectedMu.Lock()
	// check if dev exist in connectedDevices map, if exist, remove device
	if connectedDevices, ok := _bt.connectedDevices[disconnectedDev.AdapterPath]; ok {
		var tempDevices []*device
		for _, dev := range connectedDevices {
			// check if disconnected device exist in connected devices, if exist, abandon this
			if dev.Address != disconnectedDev.Address {
				tempDevices = append(tempDevices, dev)
			}
		}
		_bt.connectedDevices[disconnectedDev.AdapterPath] = tempDevices
	}
	b.connectedMu.Unlock()
}

func (b *SysBluetooth) getConnectedDeviceByAddress(address string) *device {
	b.connectedMu.Lock()
	defer b.connectedMu.Unlock()

	for _, devices := range b.connectedDevices {
		for _, dev := range devices {
			if dev.Address == address {
				return dev
			}
		}
	}

	return nil
}

func (b *SysBluetooth) findFirstConnectedDeviceByIcon(icon string) *device {
	b.connectedMu.Lock()
	defer b.connectedMu.Unlock()

	for _, devices := range b.connectedDevices {
		for _, dev := range devices {
			if dev.Icon == icon {
				return dev
			}
		}
	}
	return nil
}

func (b *SysBluetooth) setAutoConnectFinishedStatus(adapterPath dbus.ObjectPath, status bool) {
	adapter, err := b.getAdapter(adapterPath)
	if err != nil {
		// 可能是适配器被移除了
		logger.Warningf("call getAdapter failed; adapterPath:[%s] err:[%s]", adapterPath, err)
		return
	}

	if !adapter.Powered {
		// 适配器电源关闭了
		logger.Warningf("adapter： [%s] is power off", adapterPath)
		return
	}
	adapter.autoConnectFinished = status

	return
}

func (b *SysBluetooth) stopDiscovery() {
	b.adaptersMu.Lock()
	defer b.adaptersMu.Unlock()

	for _, adapter := range b.adapters {
		if adapter.Discovering {
			err := adapter.core.Adapter().StopDiscovery(0)
			if err != nil {
				logger.Warning(err)
			}
		}
	}

	return
}
