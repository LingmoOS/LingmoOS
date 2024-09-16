// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"encoding/json"
	"errors"
	"fmt"

	"os"
	"strings"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	btcommon "github.com/linuxdeepin/dde-daemon/common/bluetooth"
	audio "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.audio1"
	mpris2 "github.com/linuxdeepin/go-dbus-factory/session/org.mpris.mediaplayer2"
	obex "github.com/linuxdeepin/go-dbus-factory/system/org.bluez.obex"
	airplanemode "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.airplanemode1"
	sysbt "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.bluetooth1"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
)

const (
	dbusServiceName = "org.deepin.dde.Bluetooth1"
	dbusPath        = "/org/deepin/dde/Bluetooth1"
	dbusInterface   = dbusServiceName
	configManagerId = "org.desktopspec.ConfigManager"
)

const (
	bluetoothSchema = "com.deepin.dde.bluetooth"
	displaySwitch   = "display-switch"
)

// nolint
const (
	transferStatusQueued    = "queued"
	transferStatusActive    = "active"
	transferStatusSuspended = "suspended"
	transferStatusComplete  = "complete"
	transferStatusError     = "error"
)

//go:generate dbusutil-gen -type Bluetooth bluetooth.go
//go:generate dbusutil-gen em -type Bluetooth,agent,obexAgent

type Bluetooth struct {
	service       *dbusutil.Service
	sysBt         sysbt.Bluetooth
	sigLoop       *dbusutil.SignalLoop
	systemSigLoop *dbusutil.SignalLoop
	sysDBusDaemon ofdbus.DBus
	agent         *agent
	obexAgent     *obexAgent
	obexManager   obex.Manager

	// airplane
	airplaneBltOriginState map[dbus.ObjectPath]bool
	airplane               airplanemode.AirplaneMode

	adapters AdapterInfos
	devices  DeviceInfoMap

	initiativeConnectMap *initiativeConnectMap

	PropsMu       sync.RWMutex
	State         uint32 // StateUnavailable/StateAvailable/StateConnected
	Transportable bool   //能否传输 True可以传输 false不能传输
	CanSendFile   bool

	sessionCancelChMap   map[dbus.ObjectPath]chan struct{}
	sessionCancelChMapMu sync.Mutex

	settings *gio.Settings
	//dbusutil-gen: ignore
	DisplaySwitch gsprop.Bool `prop:"access:rw"`

	sessionCon   *dbus.Conn
	sessionAudio audio.Audio

	configManagerPath dbus.ObjectPath
	// nolint
	signals *struct {
		// adapter/device properties changed signals
		AdapterAdded, AdapterRemoved, AdapterPropertiesChanged struct {
			adapterJSON string
		}

		DeviceAdded, DeviceRemoved, DevicePropertiesChanged struct {
			devJSON string
		}

		// pair request signals
		DisplayPinCode struct {
			device  dbus.ObjectPath
			pinCode string
		}
		DisplayPasskey struct {
			device  dbus.ObjectPath
			passkey uint32
			entered uint32
		}

		// RequestConfirmation you should call Confirm with accept
		RequestConfirmation struct {
			device  dbus.ObjectPath
			passkey string
		}

		// RequestAuthorization you should call Confirm with accept
		RequestAuthorization struct {
			device dbus.ObjectPath
		}

		// RequestPinCode you should call FeedPinCode with accept and key
		RequestPinCode struct {
			device dbus.ObjectPath
		}

		// RequestPasskey you should call FeedPasskey with accept and key
		RequestPasskey struct {
			device dbus.ObjectPath
		}

		Cancelled struct {
			device dbus.ObjectPath
		}

		ObexSessionCreated struct {
			sessionPath dbus.ObjectPath
		}

		ObexSessionRemoved struct {
			sessionPath dbus.ObjectPath
		}

		ObexSessionProgress struct {
			sessionPath dbus.ObjectPath
			totalSize   uint64
			transferred uint64
			currentIdx  int
		}

		TransferCreated struct {
			file         string
			transferPath dbus.ObjectPath
			sessionPath  dbus.ObjectPath
		}

		TransferRemoved struct {
			file         string
			transferPath dbus.ObjectPath
			sessionPath  dbus.ObjectPath
			done         bool
		}
		TransferFailed struct {
			file        string
			sessionPath dbus.ObjectPath
			errInfo     string
		}
	}
}

func newBluetooth(service *dbusutil.Service) (b *Bluetooth) {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning(err)
		return nil
	}

	b = &Bluetooth{
		service:       service,
		sigLoop:       dbusutil.NewSignalLoop(service.Conn(), 0),
		systemSigLoop: dbusutil.NewSignalLoop(sysBus, 10),
		obexManager:   obex.NewManager(service.Conn()),
		Transportable: true,
	}

	b.sysBt = sysbt.NewBluetooth(sysBus)
	b.devices.infos = make(map[dbus.ObjectPath]DeviceInfos)
	b.initiativeConnectMap = newInitiativeConnectMap()
	// create airplane mode
	b.airplane = airplanemode.NewAirplaneMode(sysBus)

	b.sessionCon, err = dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return nil
	}
	b.sessionAudio = audio.NewAudio(b.sessionCon)

	// 加载dsg配置
	systemConnObj := sysBus.Object(configManagerId, "/")
	err = systemConnObj.Call(configManagerId+".acquireManager", 0, "org.deepin.dde.daemon", "org.deepin.dde.daemon.bluetooth", "").Store(&b.configManagerPath)
	if err != nil {
		logger.Warning(err)
		return nil
	}

	err = dbusutil.NewMatchRuleBuilder().Type("signal").
		PathNamespace(string(b.configManagerPath)).
		Interface("org.desktopspec.ConfigManager.Manager").
		Member("valueChanged").Build().AddTo(sysBus)
	if err != nil {
		logger.Warning(err)
		return nil
	}

	return
}

func (b *Bluetooth) destroy() {
	b.agent.destroy()
	b.sysDBusDaemon.RemoveHandler(proxy.RemoveAllHandlers)

	err := b.service.StopExport(b)
	if err != nil {
		logger.Warning(err)
	}
	b.systemSigLoop.Stop()
}

func (*Bluetooth) GetInterfaceName() string {
	return dbusInterface
}

func (b *Bluetooth) init() {
	b.sigLoop.Start()
	b.systemSigLoop.Start()
	systemBus := b.systemSigLoop.Conn()
	b.sessionCancelChMap = make(map[dbus.ObjectPath]chan struct{})

	// start bluetooth goroutine
	// monitor click signal or time out signal to close notification window
	go beginTimerNotify(globalTimerNotifier)

	b.sysBt.InitSignalExt(b.systemSigLoop, true)
	canSendFile, err := b.sysBt.CanSendFile().Get(0)
	if err != nil {
		logger.Warning(err)
	}

	configCanSendFile := b.getSendFileEnable()
	b.setPropCanSendFile(canSendFile && configCanSendFile)

	err = b.sysBt.State().ConnectChanged(func(hasValue bool, value uint32) {
		if !hasValue {
			return
		}
		b.setPropState(value)
	})
	if err != nil {
		logger.Warning(err)
	}
	state, err := b.sysBt.State().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	b.setPropState(state)

	sysBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning(err)
		return
	}

	// 加载dsg配置
	systemConnObj := sysBus.Object(configManagerId, "/")
	err = systemConnObj.Call(configManagerId+".acquireManager", 0, "org.deepin.dde.daemon", "org.deepin.dde.daemon.bluetooth", "").Store(&b.configManagerPath)
	if err != nil {
		logger.Warning(err)
	}

	_, err = b.sysBt.ConnectAdapterAdded(func(adapterJSON string) {
		adapterInfo, err := unmarshalAdapterInfo(adapterJSON)
		if err != nil {
			logger.Warning(err)
			return
		}

		b.adapters.addOrUpdateAdapter(adapterInfo)
		err = b.service.Emit(b, "AdapterAdded", adapterJSON)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	_, err = b.sysBt.ConnectAdapterRemoved(func(adapterJSON string) {
		adapterInfo, err := unmarshalAdapterInfo(adapterJSON)
		if err != nil {
			logger.Warning(err)
			return
		}

		err = b.handleBluezPort(false)
		if err != nil {
			logger.Warning(err)
		}

		b.adapters.removeAdapter(adapterInfo.Path)
		err = b.service.Emit(b, "AdapterRemoved", adapterJSON)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	_, err = b.sysBt.ConnectAdapterPropertiesChanged(func(adapterJSON string) {
		adapterInfo, err := unmarshalAdapterInfo(adapterJSON)
		if err != nil {
			logger.Warning(err)
			return
		}

		b.adapters.addOrUpdateAdapter(adapterInfo)
		err = b.service.Emit(b, "AdapterPropertiesChanged", adapterJSON)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	// 初始化 b.adapters
	adaptersJSON, err := b.sysBt.GetAdapters(0)
	if err == nil {
		var adapterInfos []AdapterInfo
		err := json.Unmarshal([]byte(adaptersJSON), &adapterInfos)
		if err == nil {
			b.adapters.mu.Lock()
			b.adapters.infos = adapterInfos
			b.adapters.mu.Unlock()
			for _, adapterInfo := range adapterInfos {
				data, err := json.Marshal(adapterInfo)
				if err != nil {
					logger.Warning(err)
					continue
				}
				err = b.service.Emit(b, "AdapterAdded", string(data))
				if err != nil {
					logger.Warning(err)
					continue
				}
			}
		} else {
			logger.Warning(err)
		}
	} else {
		logger.Warning(err)
	}

	_, err = b.sysBt.ConnectDeviceAdded(func(deviceJSON string) {
		devInfo, err := unmarshalDeviceInfo(deviceJSON)
		if err != nil {
			logger.Warning(err)
		}
		logger.Debug("DeviceAdded", devInfo.Alias, devInfo.Path)
		b.devices.addOrUpdateDevice(devInfo)
		err = b.service.Emit(b, "DeviceAdded", deviceJSON)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	_, err = b.sysBt.ConnectDeviceRemoved(func(deviceJSON string) {
		devInfo, err := unmarshalDeviceInfo(deviceJSON)
		if err != nil {
			logger.Warning(err)
		}
		logger.Debug("DeviceRemoved", devInfo.Alias, devInfo.Path)
		b.initiativeConnectMap.del(devInfo.Path)
		b.devices.removeDevice(devInfo.AdapterPath, devInfo.Path)
		err = b.service.Emit(b, "DeviceRemoved", deviceJSON)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	_, err = b.sysBt.ConnectDevicePropertiesChanged(func(deviceJSON string) {
		devInfo, err := unmarshalDeviceInfo(deviceJSON)
		if err != nil {
			logger.Warning(err)
		}

		b.devices.addOrUpdateDevice(devInfo)
		err = b.service.Emit(b, "DevicePropertiesChanged", deviceJSON)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	// 初始化 b.devices
	var adapterPaths []dbus.ObjectPath
	b.adapters.mu.Lock()
	for _, info := range b.adapters.infos {
		adapterPaths = append(adapterPaths, info.Path)
	}
	b.adapters.mu.Unlock()

	for _, adapterPath := range adapterPaths {
		devicesJSON, err := b.sysBt.GetDevices(0, adapterPath)
		if err == nil {
			var devices DeviceInfos
			err = json.Unmarshal([]byte(devicesJSON), &devices)
			if err == nil {
				b.devices.mu.Lock()
				b.devices.infos[adapterPath] = devices
				b.devices.mu.Unlock()
			} else {
				logger.Warning(err)
			}

		} else {
			logger.Warning(err)
		}

	}

	b.sysDBusDaemon = ofdbus.NewDBus(systemBus)
	b.sysDBusDaemon.InitSignalExt(b.systemSigLoop, true)
	_, err = b.sysDBusDaemon.ConnectNameOwnerChanged(b.handleDBusNameOwnerChanged)
	if err != nil {
		logger.Warning(err)
	}
	b.settings = gio.NewSettings(bluetoothSchema)
	b.DisplaySwitch.Bind(b.settings, displaySwitch)

	b.agent.init()
	b.obexAgent.init()
}

func getMprisPlayers(sessionConn *dbus.Conn) ([]string, error) {
	var playerNames []string
	dbusDaemon := ofdbus.NewDBus(sessionConn)
	names, err := dbusDaemon.ListNames(0)
	if err != nil {
		return nil, err
	}
	for _, name := range names {
		if strings.HasPrefix(name, "org.mpris.MediaPlayer2") {
			// is mpris player
			playerNames = append(playerNames, name)
		}
	}
	return playerNames, nil
}

// true ： play; false : pause
func setAllPlayers(value bool) {
	sessionConn, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return
	}
	playerNames, err := getMprisPlayers(sessionConn)
	if err != nil {
		logger.Warning("getMprisPlayers failed:", err)
		return
	}

	logger.Debug("pause all players")
	for _, playerName := range playerNames {
		player := mpris2.NewMediaPlayer(sessionConn, playerName)
		if value {
			err := player.Player().Play(0)
			if err != nil {
				logger.Warningf("failed to pause player %s: %v", playerName, err)
			}
		} else {
			err := player.Player().Pause(0)
			if err != nil {
				logger.Warningf("failed to pause player %s: %v", playerName, err)
			}
		}

	}
}

// 获取当前是否为蓝牙端口音频，是：暂停音乐
func (b *Bluetooth) handleBluezPort(value bool) error {
	//get defaultSink Name
	sinkPath, err := b.sessionAudio.DefaultSink().Get(0)
	if err != nil {
		return err
	}

	sink, err := audio.NewSink(b.sessionCon, sinkPath)
	if err != nil {
		return err
	}
	sinkName, err := sink.Name().Get(0)
	if err != nil {
		return err
	}
	isBluePort := strings.Contains(strings.ToLower(sinkName), "blue")
	logger.Info(" handleBluezPort sinkName : ", sinkName, isBluePort)
	if isBluePort {
		//stop music
		go setAllPlayers(value)
	}
	return nil
}

func (b *Bluetooth) handleDBusNameOwnerChanged(name, oldOwner, newOwner string) {
	if name != b.sysBt.ServiceName_() {
		return
	}
	if newOwner != "" {
		logger.Info("sys bluetooth is starting")
		time.AfterFunc(1*time.Second, func() {
			b.agent.register()
		})
	} else {
		logger.Info("sys bluetooth stopped")
		b.devices.clear()
		b.adapters.clear()
	}
}

type initiativeConnectMap struct {
	mu sync.Mutex
	m  map[dbus.ObjectPath]bool
}

func newInitiativeConnectMap() *initiativeConnectMap {
	return &initiativeConnectMap{
		m: make(map[dbus.ObjectPath]bool),
	}
}

func (icm *initiativeConnectMap) set(path dbus.ObjectPath, val bool) {
	icm.mu.Lock()
	defer icm.mu.Unlock()
	icm.m[path] = val
}

func (icm *initiativeConnectMap) get(path dbus.ObjectPath) bool {
	icm.mu.Lock()
	defer icm.mu.Unlock()
	return icm.m[path]
}

func (icm *initiativeConnectMap) del(path dbus.ObjectPath) {
	icm.mu.Lock()
	defer icm.mu.Unlock()
	delete(icm.m, path)
}

func (b *Bluetooth) getDevice(devPath dbus.ObjectPath) (*DeviceInfo, error) {
	info := b.devices.getDeviceWithPath(devPath)
	if info == nil {
		return nil, errors.New("device not found")
	}
	return info, nil
}

func (b *Bluetooth) feed(devPath dbus.ObjectPath, accept bool, key string) (err error) {
	_, err = b.getDevice(devPath)
	if nil != err {
		logger.Warningf("FeedRequest can not find device: %v, %v", devPath, err)
		return err
	}

	b.agent.mu.Lock()
	if b.agent.requestDevice != devPath {
		b.agent.mu.Unlock()
		logger.Warningf("FeedRequest can not find match device: %q, %q", b.agent.requestDevice, devPath)
		return btcommon.ErrCanceled
	}
	b.agent.mu.Unlock()

	select {
	case b.agent.rspChan <- authorize{path: devPath, accept: accept, key: key}:
		return nil
	default:
		return errors.New("rspChan no reader")
	}
}

func (b *Bluetooth) getConnectedDeviceByAddress(address string) *DeviceInfo {
	devInfo := b.devices.findFirst(func(devInfo *DeviceInfo) bool {
		return devInfo.ConnectState && devInfo.Address == address
	})
	return devInfo
}

func (b *Bluetooth) getDeviceByAddress(address string) *DeviceInfo {
	devInfo := b.devices.findFirst(func(devInfo *DeviceInfo) bool {
		return devInfo.Address == address
	})
	return devInfo
}

func (b *Bluetooth) sendFiles(dev *DeviceInfo, files []string) (dbus.ObjectPath, error) {
	var totalSize uint64

	for _, f := range files {
		info, err := os.Stat(f)
		if err != nil {
			return "/", err
		}

		totalSize += uint64(info.Size())
	}
	// 创建 OBEX session
	args := make(map[string]dbus.Variant)
	_, adapter := b.adapters.getAdapter(dev.AdapterPath)
	if adapter == nil {
		return "/", fmt.Errorf("not found adapter with path: %q", dev.AdapterPath)
	}
	args["Source"] = dbus.MakeVariant(adapter.Address) // 蓝牙适配器地址
	args["Target"] = dbus.MakeVariant("opp")           // 连接方式「OPP」
	sessionPath, err := b.obexManager.Client().CreateSession(0, dev.Address, args)
	if err != nil {
		logger.Warning("failed to create obex session:", err)
		return "", err
	}
	b.emitObexSessionCreated(sessionPath)
	b.setPropTransportable(false)
	logger.Debug("Transportable", b.Transportable)

	session, err := obex.NewSession(b.service.Conn(), sessionPath)
	if err != nil {
		logger.Warning("failed to get session bus:", err)
		return "", err
	}

	go b.doSendFiles(session, files, totalSize)

	return sessionPath, nil
}

func (b *Bluetooth) doSendFiles(session obex.Session, files []string, totalSize uint64) {
	sessionPath := session.Path_()
	cancelCh := make(chan struct{})

	b.sessionCancelChMapMu.Lock()
	b.sessionCancelChMap[sessionPath] = cancelCh
	b.sessionCancelChMapMu.Unlock()

	var transferredBase uint64

	for i, f := range files {
		_, err := os.Stat(f)
		if err != nil {
			b.emitTransferFailed(f, sessionPath, err.Error())
			break
		}
		transferPath, properties, err := session.ObjectPush().SendFile(0, f)
		if err != nil {
			logger.Warningf("failed to send file: %s: %s", f, err)
			continue
		}
		logger.Infof("properties: %v", properties)

		transfer, err := obex.NewTransfer(b.service.Conn(), transferPath)
		if err != nil {
			logger.Warningf("failed to send file: %s: %s", f, err)
			continue
		}

		transfer.InitSignalExt(b.sigLoop, true)

		b.emitTransferCreated(f, transferPath, sessionPath)

		ch := make(chan bool)
		err = transfer.Status().ConnectChanged(func(hasValue bool, value string) {
			if !hasValue {
				return
			}
			// 成功或者失败，说明这个传输结束
			if value == transferStatusComplete || value == transferStatusError {
				ch <- value == transferStatusComplete
			}
		})
		if err != nil {
			logger.Warning("connect to status changed failed:", err)
		}

		err = transfer.Transferred().ConnectChanged(func(hasValue bool, value uint64) {
			if !hasValue {
				return
			}

			transferred := transferredBase + value
			b.emitObexSessionProgress(sessionPath, totalSize, transferred, i+1)
		})
		if err != nil {
			logger.Warning("connect to transferred changed failed:", err)
		}

		var res bool
		var cancel bool
		select {
		case res = <-ch:
		case <-cancelCh:
			b.sessionCancelChMapMu.Lock()
			delete(b.sessionCancelChMap, sessionPath)
			b.sessionCancelChMapMu.Unlock()

			cancel = true
			err = transfer.Cancel(0)
			if err != nil {
				logger.Warning("failed to cancel transfer:", err)
			}
		}
		transfer.RemoveAllHandlers()
		b.emitTransferRemoved(f, transferPath, sessionPath, res)

		if !res {
			break
		}

		if cancel {
			break
		}

		info, err := os.Stat(f)
		if err != nil {
			logger.Warning("failed to stat file:", err)
			break
		} else {
			transferredBase += uint64(info.Size())
		}

		b.emitObexSessionProgress(sessionPath, totalSize, transferredBase, i+1)
	}

	b.sessionCancelChMapMu.Lock()
	delete(b.sessionCancelChMap, sessionPath)
	b.sessionCancelChMapMu.Unlock()

	b.emitObexSessionRemoved(sessionPath)
	b.setPropTransportable(true)

	objs, err := obex.NewObjectManager(b.service.Conn()).GetManagedObjects(0)
	if err != nil {
		logger.Warning("failed to get managed objects:", err)
	} else {
		_, pathExists := objs[sessionPath]
		if !pathExists {
			logger.Debugf("session %s not exists", sessionPath)
			return
		}
	}

	err = b.obexManager.Client().RemoveSession(0, sessionPath)
	if err != nil {
		logger.Warning("failed to remove session:", err)
	}
}

func (b *Bluetooth) emitObexSessionCreated(sessionPath dbus.ObjectPath) {
	err := b.service.Emit(b, "ObexSessionCreated", sessionPath)
	if err != nil {
		logger.Warning("failed to emit ObexSessionCreated:", err)
	}
}

func (b *Bluetooth) emitObexSessionRemoved(sessionPath dbus.ObjectPath) {
	err := b.service.Emit(b, "ObexSessionRemoved", sessionPath)
	if err != nil {
		logger.Warning("failed to emit ObexSessionRemoved:", err)
	}
}

func (b *Bluetooth) emitObexSessionProgress(sessionPath dbus.ObjectPath, totalSize uint64, transferred uint64, currentIdx int) {
	err := b.service.Emit(b, "ObexSessionProgress", sessionPath, totalSize, transferred, currentIdx)
	if err != nil {
		logger.Warning("failed to emit ObexSessionProgress:", err)
	}
}

func (b *Bluetooth) emitTransferCreated(file string, transferPath dbus.ObjectPath, sessionPath dbus.ObjectPath) {
	err := b.service.Emit(b, "TransferCreated", file, transferPath, sessionPath)
	if err != nil {
		logger.Warning("failed to emit TransferCreated:", err)
	}
}

func (b *Bluetooth) emitTransferRemoved(file string, transferPath dbus.ObjectPath, sessionPath dbus.ObjectPath, done bool) {
	err := b.service.Emit(b, "TransferRemoved", file, transferPath, sessionPath, done)
	if err != nil {
		logger.Warning("failed to emit TransferRemoved:", err)
	}
}

func (b *Bluetooth) emitTransferFailed(file string, sessionPath dbus.ObjectPath, errInfo string) {
	err := b.service.Emit(b, "TransferFailed", file, sessionPath, errInfo)
	if err != nil {
		logger.Warning("failed to emit TransferFailed:", err)
	}
}

func (b *Bluetooth) getAirplaneBltOriginStateConfig() string {
	systemConn, err := dbus.SystemBus()
	if err != nil {
		return ""
	}
	systemConnObj := systemConn.Object("org.desktopspec.ConfigManager", b.configManagerPath)
	var value string
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, "airplaneBltOriginState").Store(&value)
	if err != nil {
		logger.Warning(err)
		return ""
	}
	return value
}

func (b *Bluetooth) setAirplaneBltOriginStateConfig(value string) {
	if value == "" {
		return
	}
	systemConn, err := dbus.SystemBus()
	if err != nil {
		return
	}
	systemConnObj := systemConn.Object("org.desktopspec.ConfigManager", b.configManagerPath)
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.setValue", 0, "airplaneBltOriginState", dbus.MakeVariant(value)).Err
	if err != nil {
		logger.Warning(err)
		return
	}

	return
}

func (b *Bluetooth) getSendFileEnable() bool {
	systemConn, err := dbus.SystemBus()
	if err != nil {
		return true
	}
	systemConnObj := systemConn.Object("org.desktopspec.ConfigManager", b.configManagerPath)
	var value bool
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, "sendFileEnable").Store(&value)
	if err != nil {
		logger.Warning(err)
		return true
	}
	return value
}
