// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"errors"
	"fmt"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	bluez "github.com/linuxdeepin/go-dbus-factory/system/org.bluez"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
)

const (
	deviceStateDisconnected = 0
	// device state is connecting or disconnecting, mark them as device state doing
	deviceStateConnecting    = 1
	deviceStateConnected     = 2
	deviceStateDisconnecting = 3
)

const (
	resourceUnavailable = "Resource temporarily unavailable"
)

type deviceState uint32

func (s deviceState) String() string {
	switch s {
	case deviceStateDisconnected:
		return "Disconnected"
	case deviceStateConnecting:
		return "Connecting"
	case deviceStateConnected:
		return "Connected"
	case deviceStateDisconnecting:
		return "Disconnecting"
	default:
		return fmt.Sprintf("Unknown(%d)", s)
	}
}

var (
	errInvalidDevicePath = fmt.Errorf("invalid device path")
)

type device struct {
	core    bluez.Device
	adapter *adapter

	Path        dbus.ObjectPath
	AdapterPath dbus.ObjectPath

	Alias            string
	Trusted          bool
	Paired           bool
	State            deviceState
	ServicesResolved bool
	ConnectState     bool

	// optional
	UUIDs   []string
	Name    string
	Icon    string
	RSSI    int16
	Address string

	Battery byte

	connected         bool
	connectedTime     time.Time
	retryConnectCount int
	agentWorking      bool
	needNotify        bool

	connectPhase      connectPhase
	disconnectPhase   disconnectPhase
	disconnectChan    chan struct{}
	mu                sync.Mutex
	pairingFailedTime time.Time

	// mark if pc or mobile request a connection
	// if is pc, then do not need to show notification window
	// else show notification window
	isInitiativeConnect bool
	// remove device when device state is connecting or disconnecting may cause blueZ crash
	// to avoid this situation, remove device only allowed when connected or disconnected finished
	needRemove         bool
	removeLock         sync.Mutex
	inputReconnectMode string
	blocked            bool
}

// 设备的备份，扫描结束3分钟后保存设备
type backupDevice struct {
	Path        dbus.ObjectPath
	AdapterPath dbus.ObjectPath

	Alias            string
	Trusted          bool
	Paired           bool
	State            deviceState
	ServicesResolved bool
	ConnectState     bool

	// optional
	UUIDs   []string
	Name    string
	Icon    string
	RSSI    int16
	Address string

	Battery byte
}

type connectPhase uint32

const (
	connectPhaseNone = iota
	connectPhaseStart
	connectPhasePairStart
	connectPhasePairEnd
	connectPhaseConnectProfilesStart
	connectPhaseConnectProfilesEnd
	connectPhaseConnectFailedSleep // 反复连接过程中,失败后的短暂延时
)

type disconnectPhase uint32

const (
	disconnectPhaseNone = iota
	disconnectPhaseStart
	disconnectPhaseDisconnectStart
	disconnectPhaseDisconnectEnd
)

func (d *device) setDisconnectPhase(value disconnectPhase) {
	d.mu.Lock()
	d.disconnectPhase = value
	d.mu.Unlock()

	switch value {
	case disconnectPhaseDisconnectStart:
		logger.Debugf("%s disconnect start", d)
	case disconnectPhaseDisconnectEnd:
		logger.Debugf("%s disconnect end", d)
	}
	d.updateState()
	d.notifyDevicePropertiesChanged()
}

func (d *device) getDisconnectPhase() disconnectPhase {
	d.mu.Lock()
	value := d.disconnectPhase
	d.mu.Unlock()
	return value
}

func (d *device) setConnectPhase(value connectPhase) {
	d.mu.Lock()
	d.connectPhase = value
	d.mu.Unlock()

	switch value {
	case connectPhasePairStart:
		logger.Debugf("%s pair start", d)
	case connectPhasePairEnd:
		logger.Debugf("%s pair end", d)

	case connectPhaseConnectProfilesStart:
		logger.Debugf("%s connect profiles start", d)
	case connectPhaseConnectProfilesEnd:
		logger.Debugf("%s connect profiles end", d)
	}

	d.updateState()
	d.notifyDevicePropertiesChanged()
	if d.Paired && d.State == deviceStateConnected && d.ConnectState && d.needNotify {
		d.needNotify = false
		notifyConnected(d.Alias)
	}
}

func (d *device) getConnectPhase() connectPhase {
	d.mu.Lock()
	value := d.connectPhase
	d.mu.Unlock()
	return value
}

func (d *device) agentWorkStart() {
	logger.Debugf("%s agent work start", d)
	d.mu.Lock()
	d.agentWorking = true
	d.mu.Unlock()
	d.updateState()
	d.notifyDevicePropertiesChanged()
}

func (d *device) agentWorkEnd() {
	logger.Debugf("%s agent work end", d)
	d.mu.Lock()
	d.agentWorking = false
	d.mu.Unlock()
	d.updateState()
	d.notifyDevicePropertiesChanged()
}

func (d *device) String() string {
	return fmt.Sprintf("device [%s] %s", d.Address, d.Alias)
}

func newDevice(systemSigLoop *dbusutil.SignalLoop, dpath dbus.ObjectPath) (d *device) {
	d = &device{Path: dpath}
	systemConn := systemSigLoop.Conn()
	d.core, _ = bluez.NewDevice(systemConn, dpath)
	d.AdapterPath, _ = d.core.Adapter().Get(0)
	d.Name, _ = d.core.Name().Get(0)
	d.Alias, _ = d.core.Alias().Get(0)
	d.Address, _ = d.core.Address().Get(0)
	d.Trusted, _ = d.core.Trusted().Get(0)
	d.Paired, _ = d.core.Paired().Get(0)
	d.connected, _ = d.core.Connected().Get(0)
	d.UUIDs, _ = d.core.UUIDs().Get(0)
	d.ServicesResolved, _ = d.core.ServicesResolved().Get(0)
	d.Icon, _ = d.core.Icon().Get(0)
	d.RSSI, _ = d.core.RSSI().Get(0)
	d.blocked, _ = d.core.Blocked().Get(0)
	d.Battery, _ = d.core.Percentage().Get(0)
	d.needNotify = true
	var err error
	d.inputReconnectMode, err = d.getInputReconnectModeRaw()
	if err != nil {
		logger.Warning(err)
	}
	d.updateState()

	// 升级后第一次进入系统，登录界面时，当之前有蓝牙连接时，打开蓝牙开关（蓝牙可被发现状态 为默认状态），否则关闭
	if d.Paired && _bt.needFixBtPoweredStatus {
		// 防止多个设备反复打开蓝牙
		adapter, err := _bt.getAdapter(d.AdapterPath)
		if err != nil {
			return
		}

		if !adapter.Powered {
			_bt.SetAdapterPowered(d.AdapterPath, true)
			_bt.needFixBtPoweredStatus = false
		}
	}

	if d.Paired && d.connected {
		d.ConnectState = true
		//切換用户时添加设备到connectedDevices列表中
		_bt.addConnectedDevice(d)
	}
	d.disconnectChan = make(chan struct{})
	d.core.InitSignalExt(systemSigLoop, true)
	d.connectProperties()
	return
}

func (d *device) destroy() {
	d.core.RemoveHandler(proxy.RemoveAllHandlers)
}

func (d *device) notifyDeviceAdded() {
	logger.Debug("notifyDeviceAdded", d.Alias, d.Path)
	err := _bt.service.Emit(_bt, "DeviceAdded", marshalJSON(d))
	if err != nil {
		logger.Warning(err)
	}
	_bt.updateState()
}

func (d *device) notifyDeviceRemoved() {
	logger.Debug("notifyDeviceRemoved", d.Alias, d.Path)
	err := _bt.service.Emit(_bt, "DeviceRemoved", marshalJSON(d))
	if err != nil {
		logger.Warning(err)
	}
	_bt.updateState()
}

func (d *device) notifyDevicePropertiesChanged() {
	err := _bt.service.Emit(_bt, "DevicePropertiesChanged", marshalJSON(d))
	if err != nil {
		logger.Warning(err)
	}
	_bt.updateState()
}

func (d *device) connectProperties() {
	err := d.core.Connected().ConnectChanged(func(hasValue bool, connected bool) {
		if !hasValue {
			return
		}
		logger.Debugf("%s Connected: %v", d, connected)
		d.connected = connected

		//音频设备主动发起连接时也断开之前的音频连接
		if d.connected && d.Paired {
			go d.audioA2DPWorkaround()
		}

		// check if device need to be removed, if is, remove device
		needRemove := d.getAndResetNeedRemove()
		if needRemove {
			// start remove device
			d.adapter.bt.removeBackupDevice(d.Path)
			err := d.adapter.core.Adapter().RemoveDevice(0, d.Path)
			if err != nil {
				logger.Warningf("failed to remove device %q from adapter %q: %v",
					d.adapter.Path, d.Path, err)
				return
			}
			return
		}

		if connected {
			d.ConnectState = true
			d.connectedTime = time.Now()
			_bt.config.setDeviceConfigConnected(d, true)
			_bt.acm.handleDeviceEvent(d)
			dev := _bt.getConnectedDeviceByAddress(d.Address)
			if dev == nil {
				_bt.addConnectedDevice(d)
				logger.Debug("connectedDevices", _bt.connectedDevices)
			}
		} else {
			//If the pairing is successful and connected, the signal will be sent when the device is disconnected
			if d.Paired && d.ConnectState {
				notifyDisconnected(d.Alias)
			}
			d.needNotify = true
			d.ConnectState = false

			// if disconnect success, remove device from map
			_bt.removeConnectedDevice(d)
			// when disconnected quickly after connecting, automatically try to connect
			sinceConnected := time.Since(d.connectedTime)
			logger.Debug("sinceConnected:", sinceConnected)
			logger.Debug("retryConnectCount:", d.retryConnectCount)

			if sinceConnected < 300*time.Millisecond {
				if d.retryConnectCount == 0 {
					go func() {
						err := d.Connect()
						if err != nil {
							logger.Warning(err)
						}
					}()
				}
				d.retryConnectCount++
			} else if sinceConnected > 2*time.Second {
				d.retryConnectCount = 0
			}

			select {
			case d.disconnectChan <- struct{}{}:
				logger.Debugf("%s disconnectChan send done", d)
			default:
			}
		}
		d.updateState()
		d.notifyDevicePropertiesChanged()

		if d.needNotify && d.Paired && d.State == deviceStateConnected && d.ConnectState {
			d.notifyConnectedChanged()
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	_ = d.core.Name().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		logger.Debugf("%s Name: %v", d, value)
		d.Name = value
		d.notifyDevicePropertiesChanged()
	})

	_ = d.core.Alias().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		d.Alias = value
		logger.Debugf("%s Alias: %v", d, value)
		d.notifyDevicePropertiesChanged()
	})

	_ = d.core.Address().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		d.Address = value
		logger.Debugf("%s Address: %v", d, value)
		d.notifyDevicePropertiesChanged()
	})

	_ = d.core.Trusted().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		d.Trusted = value
		logger.Debugf("%s Trusted: %v", d, value)
		d.notifyDevicePropertiesChanged()
	})

	_ = d.core.Paired().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		d.Paired = value
		d.updateState() // Paired属性被修改，影响到下面使用的State和ConnectState
		logger.Debugf("%s Paired: %v State: %v", d, value, d.State)

		if d.Paired && d.connected && d.State == deviceStateConnected {
			d.ConnectState = true
			dev := _bt.getConnectedDeviceByAddress(d.Address)
			if dev == nil {
				_bt.addConnectedDevice(d)
			}
		}

		if d.needNotify && d.Paired && d.State == deviceStateConnected && d.ConnectState {
			notifyConnected(d.Alias)
			d.needNotify = false
		}
		d.notifyDevicePropertiesChanged()
	})

	_ = d.core.ServicesResolved().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		d.ServicesResolved = value
		logger.Debugf("%s ServicesResolved: %v", d, value)
		d.notifyDevicePropertiesChanged()
	})

	_ = d.core.Icon().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		d.Icon = value
		logger.Debugf("%s Icon: %v", d, value)
		d.notifyDevicePropertiesChanged()
		var err error
		d.inputReconnectMode, err = d.getInputReconnectModeRaw()
		if err != nil {
			logger.Warning(err)
		}
	})

	_ = d.core.UUIDs().ConnectChanged(func(hasValue bool, value []string) {
		if !hasValue {
			return
		}
		d.UUIDs = value
		logger.Debugf("%s UUIDs: %v", d, value)
		d.notifyDevicePropertiesChanged()
	})

	_ = d.core.RSSI().ConnectChanged(func(hasValue bool, value int16) {
		if !hasValue {
			d.RSSI = 0
			logger.Debugf("%s RSSI invalidated", d)
		} else {
			d.RSSI = value
			logger.Debugf("%s RSSI: %v", d, value)
		}
		d.notifyDevicePropertiesChanged()
	})

	_ = d.core.LegacyPairing().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		logger.Debugf("%s LegacyPairing: %v", d, value)
	})

	_ = d.core.Blocked().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		logger.Debugf("%s Blocked: %v", d, value)
		d.blocked = value
	})

	_ = d.core.Percentage().ConnectChanged(func(hasValue bool, value byte) {
		if !hasValue {
			return
		}
		d.Battery = value
		logger.Debugf("%s Battery: %v", d, value)
		d.notifyDevicePropertiesChanged()
	})
}

func (d *device) notifyConnectedChanged() {
	connectPhase := d.getConnectPhase()
	if connectPhase != connectPhaseNone {
		// connect is in progress
		logger.Debugf("%s handleNotifySend: connect is in progress", d)
		return
	}

	disconnectPhase := d.getDisconnectPhase()
	if disconnectPhase != disconnectPhaseNone {
		// disconnect is in progress
		logger.Debugf("%s handleNotifySend: disconnect is in progress", d)
		return
	}

	if d.connected {
		notifyConnected(d.Alias)
		d.needNotify = false
		//} else {
		//	if time.Since(d.pairingFailedTime) < 2*time.Second {
		//		return
		//	}
		//	notifyDisconnected(d.Alias)
	}
}

func (d *device) updateState() {
	newState := d.getState()
	if d.State != newState {
		d.State = newState
		logger.Debugf("%s State: %s", d, d.State)
	}
}

func (d *device) getState() deviceState {
	d.mu.Lock()
	defer d.mu.Unlock()
	if d.agentWorking {
		return deviceStateConnecting
	}

	if d.connectPhase != connectPhaseNone {
		return deviceStateConnecting

	} else if d.disconnectPhase != connectPhaseNone {
		return deviceStateDisconnecting

	} else {
		if d.connected && d.Paired {
			return deviceStateConnected
		} else {
			return deviceStateDisconnected
		}
	}
}

func (d *device) getAddress() string {
	if d.adapter != nil {
		return d.adapter.Address + "/" + d.Address
	}
	return "/"
}

func (d *device) doConnect(hasNotify bool) error {
	connectPhase := d.getConnectPhase()
	disconnectPhase := d.getDisconnectPhase()
	if connectPhase != connectPhaseNone {
		logger.Warningf("%s connect is in progress", d)
		return nil
	} else if disconnectPhase != disconnectPhaseNone {
		logger.Debugf("%s disconnect is in progress", d)
		return nil
	}

	d.setConnectPhase(connectPhaseStart)
	defer d.setConnectPhase(connectPhaseNone)

	err := d.cancelBlock()
	if err != nil {
		// if hasNotify {
		// 	// TODO(jouyouyun): notify device blocked
		// }
		return err
	}

	err = d.doPair()
	if err != nil {
		d.ConnectState = false
		if hasNotify {
			notifyConnectFailedHostDown(d.Alias)
		}
		return err
	}

	d.audioA2DPWorkaround()

	err = d.doRealConnect()
	if err != nil {
		if hasNotify {
			if resourceUnavailable == err.Error() {
				notifyConnectFailedResourceUnavailable(d.Alias, d.adapter.Alias)
			} else {
				notifyConnectFailedHostDown(d.Alias)
			}
		}
		d.ConnectState = false
		return err
	}

	d.ConnectState = true
	d.notifyDevicePropertiesChanged()
	if d.needNotify && d.Paired && d.State == deviceStateConnected && d.ConnectState {
		notifyConnected(d.Alias)
		d.needNotify = false
	}
	return nil
}

func (d *device) doRealConnect() error {
	if d.adapter.Discovering {
		err := d.adapter.core.Adapter().StopDiscovery(0)
		if err != nil {
			logger.Warning(err)
		}
		defer func() {
			err = d.adapter.core.Adapter().StartDiscovery(0)
			if err != nil {
				logger.Warning(err)
			}
		}()
	}
	d.setConnectPhase(connectPhaseConnectProfilesStart)
	err := d.core.Connect(0)
	d.setConnectPhase(connectPhaseConnectProfilesEnd)
	if err != nil {
		if strings.Contains(err.Error(), "Input/output error") {
			logger.Info("Input/output error -> ignore profile fail.")
		} else {
			// connect failed
			logger.Warningf("%s connect failed: %v", d, err)
			_bt.config.setDeviceConfigConnected(d, false)
			return err
		}
	}

	// connect succeeded
	logger.Infof("%s connect succeeded", d)
	_bt.config.setDeviceConfigConnected(d, true)

	// auto trust device when connecting success
	err = d.doTrust()
	if err != nil {
		logger.Warning(err)
	}

	return nil
}

func (d *device) doTrust() error {
	trusted, _ := d.core.Trusted().Get(0)
	if trusted {
		return nil
	}
	err := d.core.Trusted().Set(0, true)
	if err != nil {
		logger.Warning(err)
	}
	return err
}

func (d *device) cancelBlock() error {
	blocked, err := d.core.Blocked().Get(0)
	if err != nil {
		logger.Warning(err)
		return err
	}
	if !blocked {
		return nil
	}
	err = d.core.Blocked().Set(0, false)
	if err != nil {
		logger.Warning(err)
	}
	return err
}

func (d *device) cancelPairing() error {
	err := d.core.CancelPairing(0)

	return err
}

func (d *device) doPair() error {
	paired, err := d.core.Paired().Get(0)
	if err != nil {
		logger.Warning(err)
		return err
	}
	if paired {
		logger.Debugf("%s already paired", d)
		return nil
	}

	d.setConnectPhase(connectPhasePairStart)
	err = d.core.Pair(0)
	d.setConnectPhase(connectPhasePairEnd)
	if err != nil {
		logger.Warningf("%s pair failed: %v", d, err)
		d.pairingFailedTime = time.Now()
		d.setConnectPhase(connectPhaseNone)
		return err
	}

	logger.Warningf("%s pair succeeded", d)
	return nil
}

func (d *device) markNeedRemove(need bool) {
	d.removeLock.Lock()
	d.needRemove = need
	d.removeLock.Unlock()
}

// get and reset needRemove
func (d *device) getAndResetNeedRemove() bool {
	d.removeLock.Lock()
	defer d.removeLock.Unlock()
	needRemove := d.needRemove
	// if needRemove is true, reset needRemove
	if needRemove {
		d.needRemove = false
	}
	return needRemove
}

func (d *device) audioA2DPWorkaround() {
	// TODO: remove work code if bluez a2dp is ok
	// bluez do not support muti a2dp devices
	// disconnect a2dp device before connect
	for _, uuid := range d.UUIDs {
		if uuid == A2DP_SINK_UUID {
			_bt.disconnectA2DPDeviceExcept(d)
		}
	}
}

func (d *device) Connect() error {
	logger.Debug(d, "call Connect()")
	err := d.doConnect(true)
	return err
}

func (d *device) Disconnect() {
	logger.Debugf("%s call Disconnect()", d)

	disconnectPhase := d.getDisconnectPhase()
	if disconnectPhase != disconnectPhaseNone {
		logger.Debugf("%s disconnect is in progress", d)
		return
	}

	d.setDisconnectPhase(disconnectPhaseStart)
	defer d.setDisconnectPhase(disconnectPhaseNone)

	connected, err := d.core.Connected().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}
	if !connected {
		logger.Debugf("%s not connected", d)
		return
	}

	// 如果是 LE 或由设备主动重连接的设备, 则先设置 Trusted 为 false, 防止很快地重连接。
	if d.maybeReconnectByDevice() {
		err = d.core.Trusted().Set(0, false)
		if err != nil {
			logger.Warning("set trusted failed:", err)
		}
	}

	_bt.config.setDeviceConfigConnected(d, false)

	ch := d.goWaitDisconnect()

	d.setDisconnectPhase(disconnectPhaseDisconnectStart)
	err = d.core.Disconnect(0)
	if err != nil {
		logger.Warningf("failed to disconnect %s: %v", d, err)
	}
	d.setDisconnectPhase(disconnectPhaseDisconnectEnd)
	d.ConnectState = false
	d.notifyDevicePropertiesChanged()

	<-ch
	notifyDisconnected(d.Alias)
	d.needNotify = true
}

func (d *device) maybeReconnectByDevice() bool {
	reconnectMode, err := d.getInputReconnectMode()
	if err != nil {
		logger.Warning(err)
	}
	if (reconnectMode == inputReconnectModeDevice || reconnectMode == inputReconnectModeAny) ||
		!d.isBREDRDevice() {
		return true
	}
	return false
}

func (d *device) shouldReconnectByHost() bool {
	reconnectMode, err := d.getInputReconnectMode()
	if err != nil {
		logger.Warning(err)
	}
	if reconnectMode == inputReconnectModeDevice {
		return false
	}
	if (reconnectMode == inputReconnectModeHost || reconnectMode == inputReconnectModeAny) ||
		d.isBREDRDevice() {
		return true
	}
	return false
}

// nolint
const (
	inputReconnectModeNone   = "none"
	inputReconnectModeHost   = "host"
	inputReconnectModeDevice = "device"
	inputReconnectModeAny    = "any"
)

func (d *device) getInputReconnectMode() (string, error) {
	if d.inputReconnectMode != "" {
		return d.inputReconnectMode, nil
	}
	return d.getInputReconnectModeRaw()
}

func (d *device) getInputReconnectModeRaw() (string, error) {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return "", err
	}
	obj := sysBus.Object(d.core.ServiceName_(), d.core.Path_())
	if strings.HasPrefix(d.Icon, "input") {
		propVar, err := obj.GetProperty("org.bluez.Input1.ReconnectMode")
		if err != nil {
			busErr, ok := err.(dbus.Error)
			if ok && strings.Contains(strings.ToLower(busErr.Error()), "no such interface") {
				// 这个接口不是一定存在的，所以忽略这种错误。
				return "", nil
			}
			return "", err
		}
		mode, ok := propVar.Value().(string)
		if !ok {
			return "", errors.New("type of the property value is not string")
		}
		return mode, nil
	}
	return "", nil
}

func (d *device) goWaitDisconnect() chan struct{} {
	ch := make(chan struct{})
	go func() {
		select {
		case <-d.disconnectChan:
			logger.Debugf("%s disconnectChan receive ok", d)
		case <-time.After(60 * time.Second):
			logger.Debugf("%s disconnectChan receive timed out", d)
		}
		ch <- struct{}{}
	}()
	return ch
}

func (d *device) getTechnologies() ([]string, error) {
	if d.adapter == nil {
		return nil, errors.New("d.adapter is nil")
	}
	var filename = filepath.Join(bluetoothPrefixDir, d.adapter.Address, d.Address, "info")
	techs, err := doGetDeviceTechnologies(filename)
	return techs, err
}

// 按条件过滤出期望的设备，fn 返回 true 表示需要。
func filterOutDevices(devices []*device, fn func(d *device) bool) (result []*device) {
	for _, d := range devices {
		if fn(d) {
			result = append(result, d)
		}
	}
	return
}

func newBackupDevice(d *device) (bd *backupDevice) {
	bd = &backupDevice{}
	bd.AdapterPath = d.AdapterPath
	bd.Path = d.Path
	bd.Alias = d.Alias
	bd.Paired = d.Paired
	bd.Address = d.Address
	bd.State = d.State
	bd.Name = d.Name
	bd.ConnectState = d.ConnectState
	bd.Icon = d.Icon
	bd.RSSI = d.RSSI
	bd.ServicesResolved = d.ServicesResolved
	bd.Trusted = d.Trusted
	bd.UUIDs = d.UUIDs
	bd.Battery = d.Battery
	return bd
}

func (d *backupDevice) notifyDeviceRemoved() {
	logger.Debug("backupDevice notifyDeviceRemoved", d.Alias, d.Path)
	err := _bt.service.Emit(_bt, "DeviceRemoved", marshalJSON(d))
	if err != nil {
		logger.Warning(err)
	}
}
