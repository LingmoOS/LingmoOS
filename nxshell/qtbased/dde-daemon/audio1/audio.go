// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"errors"
	"fmt"
	"math"
	"sort"
	"strings"
	"sync"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/common/dsync"
	notifications "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.notifications"
	systemd1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.systemd1"
	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"
	. "github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/pulse"
	"golang.org/x/xerrors"
)

const (
	gsSchemaAudio                 = "com.deepin.dde.audio"
	gsKeyFirstRun                 = "first-run"
	gsKeyInputVolume              = "input-volume"
	gsKeyOutputVolume             = "output-volume"
	gsKeyHeadphoneOutputVolume    = "headphone-output-volume"
	gsKeyHeadphoneUnplugAutoPause = "headphone-unplug-auto-pause"
	gsKeyVolumeIncrease           = "volume-increase"

	gsKeyReduceNoise              = "reduce-input-noise"
	gsKeyOutputAutoSwitchCountMax = "output-auto-switch-count-max"

	gsSchemaControlCenter = "com.deepin.dde.control-center"
	gsKeyDeviceManager    = "device-manage"

	gsSchemaSoundEffect  = "com.deepin.dde.sound-effect"
	gsKeyEnabled         = "enabled"
	gsKeyDisableAutoMute = "disable-auto-mute"

	dbusServiceName = "org.deepin.dde.Audio1"
	dbusPath        = "/org/deepin/dde/Audio1"
	dbusInterface   = dbusServiceName

	pulseaudioService    = "pulseaudio.service"
	pipewireService      = "pipewire.service"
	pipewirePulseService = "pipewire-pulse.service"

	pulseaudioSocket    = "pulseaudio.socket"
	pipewireSocket      = "pipewire.socket"
	pipewirePulseSocket = "pipewire-pulse.socket"

	increaseMaxVolume = 1.5
	normalMaxVolume   = 1.0

	dsgkeyPausePlayer             = "pausePlayer"
	dsgKeyAutoSwitchPort          = "autoSwitchPort"
	dsgKeyBluezModeFilterList     = "bluezModeFilterList"
	dsgKeyPortFilterList          = "portFilterList"
	dsgKeyReduceNoise             = "reduceNoise"
	dsgKeyInputDefaultPriorities  = "inputDefaultPrioritiesByType"
	dsgKeyOutputDefaultPriorities = "outputDefaultPrioritiesByType"
	dsgKeyBluezModeDefault        = "bluezModeDefault"

	changeIconStart    = "notification-change-start"
	changeIconFailed   = "notification-change-failed"
	changeIconFinished = "notification-change-finished"
)

var (
	defaultInputVolume           = 0.1
	defaultOutputVolume          = 0.5
	defaultHeadphoneOutputVolume = 0.17
	gMaxUIVolume                 float64
	defaultReduceNoise           = false

	// 保存 pulaudio ,pipewire 相关的服务
	pulseaudioServices = []string{pulseaudioService, pulseaudioSocket}
	pipewireServices   = []string{pipewireService, pipewirePulseService, pipewireSocket, pipewirePulseSocket}
)

const (
	// 音频服务更改状态：已经完成
	AudioStateChanged = true
	// 音频服务更改状态：正在修改中
	AudioStateChanging = false
)

//go:generate dbusutil-gen -type Audio,Sink,SinkInput,Source,Meter -import github.com/godbus/dbus audio.go sink.go sinkinput.go source.go meter.go
//go:generate dbusutil-gen em -type Audio,Sink,SinkInput,Source,Meter

func objectPathSliceEqual(v1, v2 []dbus.ObjectPath) bool {
	if len(v1) != len(v2) {
		return false
	}
	for i, e1 := range v1 {
		if e1 != v2[i] {
			return false
		}
	}
	return true
}

func isStrvEqual(l1, l2 []string) bool {
	if len(l1) != len(l2) {
		return false
	}

	sort.Strings(l1)
	sort.Strings(l2)
	for i, v := range l1 {
		if v != l2[i] {
			return false
		}
	}
	return true
}

type Audio struct {
	service *dbusutil.Service
	PropsMu sync.RWMutex
	// dbusutil-gen: equal=objectPathSliceEqual
	SinkInputs []dbus.ObjectPath
	// dbusutil-gen: equal=objectPathSliceEqual
	Sinks []dbus.ObjectPath
	// dbusutil-gen: equal=objectPathSliceEqual
	configManagerPath       dbus.ObjectPath
	Sources                 []dbus.ObjectPath
	DefaultSink             dbus.ObjectPath
	DefaultSource           dbus.ObjectPath
	Cards                   string
	CardsWithoutUnavailable string
	BluetoothAudioMode      string // 蓝牙模式
	// dbusutil-gen: equal=isStrvEqual
	BluetoothAudioModeOpts []string // 可用的蓝牙模式
	CurrentAudioServer     string   // 当前使用的音频服务
	AudioServerState       bool     // 音频服务状态

	// dbusutil-gen: ignore
	IncreaseVolume gsprop.Bool `prop:"access:rw"`

	PausePlayer bool `prop:"access:rw"`

	ReduceNoise bool `prop:"access:rw"`

	defaultPaCfg defaultPaConfig

	// 最大音量
	MaxUIVolume float64 // readonly

	headphoneUnplugAutoPause bool

	settings  *gio.Settings
	ctx       *pulse.Context
	eventChan chan *pulse.Event
	stateChan chan int

	// 正常输出声音的程序列表
	sinkInputs        map[uint32]*SinkInput
	defaultSink       *Sink
	defaultSource     *Source
	sinks             map[uint32]*Sink
	sources           map[uint32]*Source
	defaultSinkName   string
	defaultSourceName string
	meters            map[string]*Meter
	mu                sync.Mutex
	quit              chan struct{}

	oldCards CardList // cards在上次更新前的状态，用于判断Port是否是新插入的
	cards    CardList

	isSaving     bool
	sourceIdx    uint32 // used to disable source if select a2dp profile
	saverLocker  sync.Mutex
	enableSource bool // can not enable a2dp Source if card profile is "a2dp"

	portLocker sync.Mutex

	syncConfig     *dsync.Config
	sessionSigLoop *dbusutil.SignalLoop

	noRestartPulseAudio bool

	// 当前输入端口
	inputCardName string
	inputPortName string
	// 输入端口切换计数器
	inputAutoSwitchCount int
	// 当前输出端口
	outputCardName string
	outputPortName string
	// 输出端口切换计数器
	outputAutoSwitchCount    int
	outputAutoSwitchCountMax int
	// 自动端口切换
	enableAutoSwitchPort    bool
	controlCenterGsSettings *gio.Settings
	// 控制中心-声音-设备管理 是否显示
	controlCenterDeviceManager gsprop.Bool `prop:"access:rw"`

	systemSigLoop *dbusutil.SignalLoop
	// 用来进一步断是否需要暂停播放的信息
	misc uint32

	// nolint
	signals *struct {
		PortEnabledChanged struct {
			cardId   uint32
			portName string
			enabled  bool
		}
	}
}

func isStringInSlice(list []string, str string) bool {
	for _, v := range list {
		if v == str {
			return true
		}
	}
	return false
}

func newAudio(service *dbusutil.Service) *Audio {
	a := &Audio{
		service:          service,
		meters:           make(map[string]*Meter),
		MaxUIVolume:      pulse.VolumeUIMax,
		enableSource:     true,
		AudioServerState: AudioStateChanged,
	}

	a.settings = gio.NewSettings(gsSchemaAudio)
	a.settings.Reset(gsKeyInputVolume)
	a.settings.Reset(gsKeyOutputVolume)
	a.settings.Reset(gsKeyHeadphoneOutputVolume)
	a.IncreaseVolume.Bind(a.settings, gsKeyVolumeIncrease)
	a.PausePlayer = false
	a.ReduceNoise = false
	a.emitPropChangedReduceNoise(a.ReduceNoise)
	a.CurrentAudioServer = a.getCurrentAudioServer()
	a.headphoneUnplugAutoPause = a.settings.GetBoolean(gsKeyHeadphoneUnplugAutoPause)
	a.outputAutoSwitchCountMax = int(a.settings.GetInt(gsKeyOutputAutoSwitchCountMax))
	if a.IncreaseVolume.Get() {
		a.MaxUIVolume = increaseMaxVolume
	} else {
		a.MaxUIVolume = normalMaxVolume
	}
	gMaxUIVolume = a.MaxUIVolume
	a.listenGSettingVolumeIncreaseChanged()

	if isStringInSlice(gio.SettingsListSchemas(), gsSchemaControlCenter) {
		a.controlCenterGsSettings = gio.NewSettings(gsSchemaControlCenter)
		if isStringInSlice(a.controlCenterGsSettings.ListKeys(), gsKeyDeviceManager) {
			a.controlCenterDeviceManager.Bind(a.controlCenterGsSettings, gsKeyDeviceManager)
			a.listenGSettingDeviceManageChanged()
		}
	} else {
		logger.Warning(" [newAudio] gschemas file not exist : /usr/share/glib-2.0/schemas/com.deepin.dde.control-center.gschema.xml")
	}

	a.sessionSigLoop = dbusutil.NewSignalLoop(service.Conn(), 10)
	a.syncConfig = dsync.NewConfig("audio", &syncConfig{a: a},
		a.sessionSigLoop, dbusPath, logger)
	a.sessionSigLoop.Start()

	return a
}

func (a *Audio) setAudioServerFailed(oldAudioServer string) {
	sendNotify(changeIconFailed, "", Tr("Failed to change Audio Server, please try later"))
	// 还原音频服务
	a.PropsMu.Lock()
	a.setPropCurrentAudioServer(oldAudioServer)
	a.setPropAudioServerState(AudioStateChanged)
	a.PropsMu.Unlock()
}

func (a *Audio) getCurrentAudioServer() (serverName string) {
	audioServers := []string{pulseaudioService, pipewireService}
	systemd := systemd1.NewManager(a.service.Conn())

	for _, server := range audioServers {
		path, err := systemd.GetUnit(0, server)
		if err == nil {
			serverSystemdUnit, err := systemd1.NewUnit(a.service.Conn(), path)
			if err == nil {
				state, err := serverSystemdUnit.Unit().LoadState().Get(0)
				if err != nil {
					logger.Warning("Failed to get LoadState of unit", path)
				} else if state == "loaded" {
					return strings.Split(server, ".")[0]
				}
			}
		}
	}

	return ""
}

func (a *Audio) SetCurrentAudioServer(serverName string) *dbus.Error {
	a.PropsMu.Lock()
	a.setPropAudioServerState(AudioStateChanging)
	a.setPropCurrentAudioServer(serverName)
	a.PropsMu.Unlock()

	sendNotify(changeIconStart, "", Tr("Changing Audio Server, please wait..."))

	var activeServices, deactiveServices []string
	if serverName == "pulseaudio" {
		activeServices = pulseaudioServices
		deactiveServices = pipewireServices
	} else {
		activeServices = pipewireServices
		deactiveServices = pulseaudioServices
	}

	oldAudioServer := a.CurrentAudioServer
	systemd := systemd1.NewManager(a.service.Conn())
	_, err := systemd.UnmaskUnitFiles(0, activeServices, false)
	if err != nil {
		logger.Warning("Failed to unmask unit files", activeServices, "\nError:", err)
		a.setAudioServerFailed(oldAudioServer)
		return dbusutil.ToError(err)
	}

	_, err = systemd.MaskUnitFiles(0, deactiveServices, false, true)
	if err != nil {
		logger.Warning("Failed to mask unit files", deactiveServices, "\nError:", err)
		a.setAudioServerFailed(oldAudioServer)
		return dbusutil.ToError(err)
	}

	err = systemd.Reload(0)
	if err != nil {
		logger.Warning("Failed to reload unit files. Error:", err)
		return dbusutil.ToError(err)
	}

	sendNotify(changeIconFinished, "", Tr("Audio Server changed, please log out and then log in"))

	a.PropsMu.Lock()
	a.setPropAudioServerState(AudioStateChanged)
	a.PropsMu.Unlock()
	return nil
}

func sendNotify(icon, summary, body string) {
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return
	}
	n := notifications.NewNotifications(sessionBus)
	_, err = n.Notify(0, Tr("dde-control-center"), 0,
		icon, summary, body,
		nil, nil, -1)
	logger.Debugf("send notification icon: %q, summary: %q, body: %q",
		icon, summary, body)

	if err != nil {
		logger.Warning(err)
	}
}

func startAudioServer(service *dbusutil.Service) error {
	var pulseaudioState string
	var activeServices, deactiveServices []string
	audioServers := []string{pulseaudioService, pipewireService}

	systemd := systemd1.NewManager(service.Conn())

	for _, server := range audioServers {
		path, err := systemd.GetUnit(0, server)
		if err == nil {
			serverSystemdUnit, err := systemd1.NewUnit(service.Conn(), path)
			if err != nil {
				logger.Warning("failed to create service systemd unit", err)
				return err
			}

			state, err := serverSystemdUnit.Unit().LoadState().Get(0)
			if err != nil {
				logger.Warning("failed to get service active state", err)
				return err
			}

			if server == pulseaudioService {
				pulseaudioState = state
			}

			// 做一个特判，pulseaudio 与 pipewire 同时预装，但是没法在装包的时候进行 mask 操作，
			if state == "loaded" && server == pulseaudioService {
				activeServices = pulseaudioServices
				deactiveServices = pipewireServices
			} else if pulseaudioState == "masked" && server == pipewireService {
				activeServices = pipewireServices
				deactiveServices = pulseaudioServices
			}

		}
	}

	for _, deactiveService := range deactiveServices {
		deactiveServicePath, err := systemd.GetUnit(0, deactiveService)
		if err == nil {
			if len(deactiveServicePath) != 0 {
				serverSystemdUnit, err := systemd1.NewUnit(service.Conn(), deactiveServicePath)

				if err != nil {
					logger.Warning("failed to create service systemd unit", err)
					return err
				}

				state, err := serverSystemdUnit.Unit().LoadState().Get(0)
				if err != nil {
					logger.Warning("failed to get service active state", err)
					return err
				}

				if state != "masked" {
					_, err := systemd.MaskUnitFiles(0, []string{deactiveService}, false, true)

					if err != nil {
						logger.Warning("Failed to mask unit files", err)
						return err
					}
				}

				// 服务在 mask 之前服务，可能被激活，调用 stop
				_, err = systemd.StopUnit(0, deactiveService, "replace")
				if err != nil {
					logger.Warning("Failed to stop service", err)
					return err
				}
			}
		}
	}

	for _, activeService := range activeServices {
		activeServicePath, err := systemd.GetUnit(0, activeService)
		if err == nil {
			logger.Debug("ready to start audio server", activeServicePath)

			if len(activeServicePath) != 0 {
				serverSystemdUnit, err := systemd1.NewUnit(service.Conn(), activeServicePath)
				if err != nil {
					logger.Warning("failed to create audio server systemd unit", err)
					return err
				}

				state, err := serverSystemdUnit.Unit().ActiveState().Get(0)
				if err != nil {
					logger.Warning("failed to get audio server active state", err)
					return err
				}

				if state != "active" {
					go func() {
						_, err := serverSystemdUnit.Unit().Start(0, "replace")
						if err != nil {
							logger.Warning("failed to start audio server unit:", err)
						}
					}()
				}
			}

		}
	}

	return nil
}

func getCtx() (ctx *pulse.Context, err error) {
	ctx = pulse.GetContextForced()
	if ctx == nil {
		err = errors.New("failed to get pulse context")
		return
	}
	return
}

const (
	dndVirtualSinkName        = "deepin_network_displays"
	fakeCardName              = "VirtualCard"
	dndVirtualSinkDescription = "NetworkDisplayDevice "
)

// genFakeCard 用于适配虚拟sink的场景,生成一个假的card
func (a *Audio) genFakeCard() *Card {
	sinkList := a.ctx.GetSinkList()
	for _, sink := range sinkList {
		if sink.Name == dndVirtualSinkName {
			port := pulse.CardPortInfo{
				PortInfo: pulse.PortInfo{
					Name:        dndVirtualSinkName,
					Description: dndVirtualSinkDescription,
					Priority:    0,
					Available:   2,
				},
				Direction: 1,
				Profiles:  nil,
			}
			card := &pulse.Card{
				Index:         sink.Index,
				Name:          fakeCardName,
				OwnerModule:   0,
				Driver:        "",
				PropList:      nil,
				Profiles:      nil,
				ActiveProfile: pulse.ProfileInfo2{},
				Ports: pulse.CardPortInfos{
					port,
				},
			}

			fakeCard := &Card{
				Id:            sink.Card,
				Name:          card.Name,
				ActiveProfile: nil,
				Profiles:      nil,
				Ports: pulse.CardPortInfos{
					port,
				},
				core: card,
			}
			return fakeCard
		}
	}
	return nil
}

func (a *Audio) refreshCards() {
	a.cards = newCardList(a.ctx.GetCardList())
	fakeCard := a.genFakeCard()
	if fakeCard != nil {
		a.cards = append(a.cards, fakeCard)
	}
	cards := a.cards.string()
	logger.Infof("cards : %s", cards)
	a.setPropCards(cards)
	a.setPropCardsWithoutUnavailable(a.cards.stringWithoutUnavailable())
}

// 添加一个新的sink,参数是pulse的Sink
func (a *Audio) addSink(sinkInfo *pulse.Sink) {
	sink := newSink(sinkInfo, a)
	a.sinks[sinkInfo.Index] = sink
	sinkPath := sink.getPath()
	err := a.service.Export(sinkPath, sink)
	if err != nil {
		logger.Warning(err)
	}
	a.updatePropSinks()
}

// 添加一个新的source,参数是pulse的Source
func (a *Audio) addSource(sourceInfo *pulse.Source) {
	source := newSource(sourceInfo, a)
	a.sources[sourceInfo.Index] = source
	sourcePath := source.getPath()
	err := a.service.Export(sourcePath, source)
	if err != nil {
		logger.Warning(err)
	}
	a.updatePropSources()
}

// 添加一个新的sink-input,参数是pulse的SinkInput
func (a *Audio) addSinkInput(sinkInputInfo *pulse.SinkInput) {
	logger.Debug("new")
	sinkInput := newSinkInput(sinkInputInfo, a)
	logger.Debug("new done")
	a.sinkInputs[sinkInputInfo.Index] = sinkInput
	sinkInputPath := sinkInput.getPath()
	err := a.service.Export(sinkInputPath, sinkInput)
	if err != nil {
		logger.Warning(err)
	}
	logger.Debug("updatePropSinkInputs")
	a.updatePropSinkInputs()
	logger.Debug("updatePropSinkInputs done")
}

func (a *Audio) refreshSinks() {
	if a.sinks == nil {
		a.sinks = make(map[uint32]*Sink)
	}

	// 获取当前的sinks
	sinkInfoMap := make(map[uint32]*pulse.Sink)
	sinkInfoList := a.ctx.GetSinkList()

	for _, sinkInfo := range sinkInfoList {
		if sinkInfo.Name == dndVirtualSinkName {
			port := pulse.PortInfo{
				Name:        sinkInfo.Name,
				Description: dndVirtualSinkDescription,
				Priority:    0,
				Available:   2,
			}
			sinkInfo.Ports = append(sinkInfo.Ports, port)
			sinkInfo.ActivePort = port
		}
		sinkInfoMap[sinkInfo.Index] = sinkInfo
		sink, exist := a.sinks[sinkInfo.Index]
		if exist {
			// 存在则更新
			logger.Debugf("update sink #%d", sinkInfo.Index)
			sink.update(sinkInfo)
		} else {
			// 不存在则添加
			logger.Debugf("add sink #%d", sinkInfo.Index)
			a.addSink(sinkInfo)
		}
	}

	// 删除不存在的旧sink
	for key, sink := range a.sinks {
		_, exist := sinkInfoMap[key]
		if !exist {
			logger.Debugf("delete sink #%d", key)
			a.service.StopExport(sink)
			delete(a.sinks, key)
		}
	}
}

func (a *Audio) refreshSources() {
	if a.sources == nil {
		a.sources = make(map[uint32]*Source)
	}

	// 获取当前的sources
	sourceInfoMap := make(map[uint32]*pulse.Source)
	sourceInfoList := a.ctx.GetSourceList()

	for _, sourceInfo := range sourceInfoList {
		sourceInfoMap[sourceInfo.Index] = sourceInfo
		source, exist := a.sources[sourceInfo.Index]
		if exist {
			// 存在则更新
			logger.Debugf("update source #%d", sourceInfo.Index)
			source.update(sourceInfo)
		} else {
			// 不存在则添加
			logger.Debugf("add source #%d", sourceInfo.Index)
			a.addSource(sourceInfo)
		}
	}

	// 删除不存在的旧source
	for key, source := range a.sources {
		_, exist := sourceInfoMap[key]
		if !exist {
			logger.Debugf("delete source #%d", key)
			a.service.StopExport(source)
			delete(a.sources, key)
		}
	}

	a.updatePropSources()
}

func (a *Audio) refershSinkInputs() {
	if a.sinkInputs == nil {
		a.sinkInputs = make(map[uint32]*SinkInput)
	}

	// 获取当前的sink-inputs
	sinkInputInfoMap := make(map[uint32]*pulse.SinkInput)
	sinkInputInfoList := a.ctx.GetSinkInputList()

	for _, sinkInputInfo := range sinkInputInfoList {
		sinkInputInfoMap[sinkInputInfo.Index] = sinkInputInfo
		sinkInput, exist := a.sinkInputs[sinkInputInfo.Index]
		if exist {
			logger.Debugf("update sink-input #%d", sinkInputInfo.Index)
			sinkInput.update(sinkInputInfo)
		} else {
			logger.Debugf("add sink-input #%d", sinkInputInfo.Index)
			a.addSinkInput(sinkInputInfo)
		}
	}

	// 删除不存在的旧sink-inputs
	for key, sinkInput := range a.sinkInputs {
		_, exist := sinkInputInfoMap[key]
		if !exist {
			logger.Debugf("delete sink-input #%d", key)
			a.service.StopExport(sinkInput)
			delete(a.sinkInputs, key)
		}
	}
}

func (a *Audio) shouldAutoPause() bool {
	if !a.PausePlayer {
		return false
	}
	if a.defaultSink == nil {
		logger.Warning("default sink is nil")
		return false
	}

	// 云平台无card
	if a.defaultSink.Card == math.MaxUint32 {
		logger.Warningf("default sink car is %d", a.defaultSink.Card)
		return false
	}

	card, err := a.cards.get(a.defaultSink.Card)
	if err != nil {
		logger.Warning(err)
		return false
	}

	port, err := card.Ports.Get(a.defaultSink.ActivePort.Name, pulse.DirectionSink)
	if err != nil {
		logger.Warning(err)
		return false
	}

	logger.Debugf("default sink active port: %v %v", port.Name, port.Available)
	if a.defaultSink.ActivePort.Available == 1 {
		logger.Warningf("default sink activePort available is %d", a.defaultSink.ActivePort.Available)
		return false
	}

	switch DetectPortType(card.core, &port) {
	case PortTypeBluetooth, PortTypeHeadset, PortTypeLineIO, PortTypeUsb:
		// 先缓存sink 是否为可插拔信息，防止后面整个card丢失，缺少判断信息。
		a.defaultSink.pluggable = true
		return true
	default:
		a.defaultSink.pluggable = false
		return false
	}
}

func (a *Audio) autoPause() {
	if !a.shouldAutoPause() {
		return
	}

	var port pulse.CardPortInfo
	card, err := a.ctx.GetCard(a.defaultSink.Card)

	if err == nil {
		port, err = card.Ports.Get(a.defaultSink.ActivePort.Name, pulse.DirectionSink)
	}

	if err != nil {
		logger.Warning(err)
		go pauseAllPlayers()
	} else if card.ActiveProfile.Name == "off" {
		go pauseAllPlayers()
	} else if port.Available == pulse.AvailableTypeNo {
		// 使用优先级并且未开启自动切换时，先不暂停，后面根据sink信息判断是否需要暂停
		logger.Debug("wait check priority", port.Priority)
		a.misc = port.Priority
		if a.misc == 0 || a.canAutoSwitchPort() {
			go pauseAllPlayers()
		}
	}
}

func (a *Audio) refreshDefaultSinkSource() {
	defaultSink := a.ctx.GetDefaultSink()
	defaultSource := a.ctx.GetDefaultSource()

	if a.defaultSink != nil && a.defaultSink.Name != defaultSink {
		logger.Debugf("update default sink to %s", defaultSink)
		if a.misc != 0 {
			a.misc = 0
			go pauseAllPlayers()
		} else if a.defaultSink.pluggable {
			// 异步状况下，可能整个card不存在(比如蓝牙)，可插拔sink切换, 需再判断下card信息。
			if _, err := a.ctx.GetCard(a.defaultSink.Card); err != nil {
				go pauseAllPlayers()
			}
		}
		a.updateDefaultSink(defaultSink)
	} else {
		logger.Debugf("keep default as %s", defaultSink)
		if a.misc != 0 {
			if card, err := a.ctx.GetCard(a.defaultSink.Card); err == nil {
				port, err := card.Ports.Get(a.defaultSink.ActivePort.Name, pulse.DirectionSink)
				if err != nil {
					logger.Warning(err)
					go pauseAllPlayers()
				} else {
					// 非可插拔sink 和 可插拔sink的port优先级变低了才暂停。
					if !a.defaultSink.pluggable ||
						port.Priority < a.misc ||
						port.Available == pulse.AvailableTypeNo {
						go pauseAllPlayers()
					}
				}
			}
			a.misc = 0
		}
	}

	if a.defaultSource != nil && a.defaultSource.Name != defaultSource {
		logger.Debugf("update default source to %s", defaultSource)
		a.updateDefaultSource(defaultSource)
	} else {
		logger.Debugf("keep default as %s", defaultSource)
	}
}

func (a *Audio) prepareRefresh() {
	a.autoPause()
}

func (a *Audio) refresh() {
	logger.Debug("prepareRefresh")
	a.prepareRefresh()
	logger.Debug("refresh cards")
	a.refreshCards()
	logger.Debug("refresh sinks")
	a.refreshSinks()
	logger.Debug("refresh sources")
	a.refreshSources()
	logger.Debug("refresh sinkinputs")
	a.refershSinkInputs()
	logger.Debug("refresh default")
	a.refreshDefaultSinkSource()
	logger.Debug("refresh bluetooth mode opts")
	a.refreshBluetoothOpts()
	logger.Debug("refresh done")
}

func (a *Audio) init() error {
	if a.settings.GetBoolean(gsKeyDisableAutoMute) {
		err := disableAutoMuteMode()
		if err != nil {
			logger.Warning(err)
		}
	}
	a.initDefaultVolumes()
	ctx, err := getCtx()
	if err != nil {
		return xerrors.Errorf("failed to get context: %w", err)
	}

	a.defaultPaCfg = loadDefaultPaConfig(defaultPaFile)
	logger.Debugf("defaultPaConfig: %+v", a.defaultPaCfg)

	a.ctx = ctx

	err = a.initDsgProp()
	if err != nil {
		return err
	}

	// 更新本地数据
	a.refresh()
	a.oldCards = a.cards
	for _, card := range a.cards {
		if isBluezAudio(card.core.Name) {
			card.AutoSetBluezMode()
		}
	}

	serverInfo, err := a.ctx.GetServer()
	if err == nil {
		a.mu.Lock()
		a.defaultSourceName = serverInfo.DefaultSourceName
		a.defaultSinkName = serverInfo.DefaultSinkName

		for _, sink := range a.sinks {
			if sink.Name == a.defaultSinkName {
				a.defaultSink = sink
				a.PropsMu.Lock()
				a.setPropDefaultSink(sink.getPath())
				a.PropsMu.Unlock()
			}
		}

		for _, source := range a.sources {
			if strings.Contains(a.defaultSourceName, "record_mono") && strings.Contains(source.Name, "alsa_input.platform-rk809-sound.analog-stereo") {
				a.defaultSource = source
				a.PropsMu.Lock()
				a.setPropDefaultSource(source.getPath())
				a.PropsMu.Unlock()
				logger.Debug("init setPropDefaultSource:", source.Name, source.getPath())
			} else {
				if source.Name == a.defaultSourceName {
					a.defaultSource = source
					a.PropsMu.Lock()
					a.setPropDefaultSource(source.getPath())
					a.PropsMu.Unlock()
				}
			}
		}
		a.mu.Unlock()
	} else {
		logger.Warning(err)
	}

	GetBluezAudioManager().Load()
	GetConfigKeeper().Load()

	logger.Debug("init cards")
	a.PropsMu.Lock()
	a.setPropCards(a.cards.string())
	a.setPropCardsWithoutUnavailable(a.cards.stringWithoutUnavailable())
	a.PropsMu.Unlock()

	a.eventChan = make(chan *pulse.Event, 100)
	a.stateChan = make(chan int, 10)
	a.quit = make(chan struct{})
	a.ctx.AddEventChan(a.eventChan)
	a.ctx.AddStateChan(a.stateChan)
	a.inputAutoSwitchCount = 0
	a.outputAutoSwitchCount = 0

	// priorities.Load(globalPrioritiesFilePath, a.cards) // TODO: 删除
	GetPriorityManager().Init(a.cards)
	GetPriorityManager().Print()

	go a.handleEvent()
	go a.handleStateChanged()
	logger.Debug("init done")

	firstRun := a.settings.GetBoolean(gsKeyFirstRun)
	if firstRun {
		logger.Info("first run, Will remove old audio config")
		removeConfig()
		a.settings.SetBoolean(gsKeyFirstRun, false)
	}

	if !a.needAutoSwitchOutputPort() {
		a.resumeSinkConfig(a.defaultSink)
	}

	if !a.needAutoSwitchInputPort() {
		a.resumeSourceConfig(a.defaultSource, isPhysicalDevice(a.defaultSourceName))
	}

	// 自动切换会在切换后触发配置恢复
	a.autoSwitchPort()

	a.fixActivePortNotAvailable()
	a.moveSinkInputsToDefaultSink()

	// 蓝牙支持的模式
	a.setPropBluetoothAudioModeOpts([]string{"a2dp", "headset", "handsfree"})

	return nil
}

func (a *Audio) destroyCtxRelated() {
	a.mu.Lock()
	a.ctx.RemoveEventChan(a.eventChan)
	a.ctx.RemoveStateChan(a.stateChan)
	close(a.quit)
	a.ctx = nil

	for _, sink := range a.sinks {
		err := a.service.StopExportByPath(sink.getPath())
		if err != nil {
			logger.Warningf("failed to stop export sink #%d: %v", sink.index, err)
		}
	}
	a.sinks = nil

	for _, source := range a.sources {
		err := a.service.StopExportByPath(source.getPath())
		if err != nil {
			logger.Warningf("failed to stop export source #%d: %v", source.index, err)
		}
	}
	a.sources = nil

	for _, sinkInput := range a.sinkInputs {
		err := a.service.StopExportByPath(sinkInput.getPath())
		if err != nil {
			logger.Warningf("failed to stop export sink input #%d: %v", sinkInput.index, err)
		}
	}
	a.sinkInputs = nil

	for _, meter := range a.meters {
		err := a.service.StopExport(meter)
		if err != nil {
			logger.Warning(err)
		}
	}
	a.mu.Unlock()
}

func (a *Audio) destroy() {
	a.settings.Unref()
	a.sessionSigLoop.Stop()
	a.systemSigLoop.Stop()
	a.syncConfig.Destroy()
	a.destroyCtxRelated()
}

func (a *Audio) initDefaultVolumes() {
	inVolumePer := float64(a.settings.GetInt(gsKeyInputVolume)) / 100.0
	outVolumePer := float64(a.settings.GetInt(gsKeyOutputVolume)) / 100.0
	headphoneOutVolumePer := float64(a.settings.GetInt(gsKeyHeadphoneOutputVolume)) / 100.0
	defaultInputVolume = inVolumePer
	defaultOutputVolume = outVolumePer
	defaultHeadphoneOutputVolume = headphoneOutVolumePer
}

func (a *Audio) findSinkByCardIndexPortName(cardId uint32, portName string) *pulse.Sink {
	for _, sink := range a.ctx.GetSinkList() {
		if isPortExists(portName, sink.Ports) && sink.Card == cardId {
			return sink
		}
	}
	return nil
}

func (a *Audio) findSourceByCardIndexPortName(cardId uint32, portName string) *pulse.Source {
	for _, source := range a.ctx.GetSourceList() {
		if isPortExists(portName, source.Ports) && source.Card == cardId {
			return source
		}
	}
	return nil
}

// set default sink and sink active port
func (a *Audio) setDefaultSinkWithPort(cardId uint32, portName string) error {
	if portName == dndVirtualSinkName || cardId == math.MaxUint32 {
		a.ctx.SetDefaultSink(portName)
		return nil
	}
	_, portConfig := GetConfigKeeper().GetCardAndPortConfig(a.getCardNameById(cardId), portName)
	if !portConfig.Enabled {
		return fmt.Errorf("card #%d port %q is disabled", cardId, portName)
	}
	logger.Debugf("setDefaultSinkWithPort card #%d port %q", cardId, portName)
	sink := a.findSinkByCardIndexPortName(cardId, portName)
	if sink == nil {
		return fmt.Errorf("cannot find valid sink for card #%d and port %q",
			cardId, portName)
	}
	if sink.ActivePort.Name != portName {
		logger.Debugf("set sink #%d port %s", sink.Index, portName)
		a.ctx.SetSinkPortByIndex(sink.Index, portName)
	}
	if a.getDefaultSinkName() != sink.Name {
		logger.Debugf("set default sink #%d %s", sink.Index, sink.Name)
		a.ctx.SetDefaultSink(sink.Name)
	}
	return nil
}

func (a *Audio) getDefaultSinkActivePortName() string {
	defaultSink := a.getDefaultSink()
	if defaultSink == nil {
		return ""
	}

	defaultSink.PropsMu.RLock()
	name := defaultSink.ActivePort.Name
	defaultSink.PropsMu.RUnlock()
	return name
}

func (a *Audio) getDefaultSourceActivePortName() string {
	defaultSource := a.getDefaultSource()
	if defaultSource == nil {
		return ""
	}

	defaultSource.PropsMu.RLock()
	name := defaultSource.ActivePort.Name
	defaultSource.PropsMu.RUnlock()
	return name
}

// set default source and source active port
func (a *Audio) setDefaultSourceWithPort(cardId uint32, portName string) error {
	_, portConfig := GetConfigKeeper().GetCardAndPortConfig(a.getCardNameById(cardId), portName)
	if !portConfig.Enabled {
		return fmt.Errorf("card #%d port %q is disabled", cardId, portName)
	}
	logger.Debugf("setDefault card #%d port %q", cardId, portName)
	source := a.findSourceByCardIndexPortName(cardId, portName)
	if source == nil {
		return fmt.Errorf("cannot find valid source for card #%d and port %q",
			cardId, portName)
	}

	if source.ActivePort.Name != portName {
		logger.Debugf("set source #%d port %s", source.Index, portName)
		a.ctx.SetSourcePortByIndex(source.Index, portName)
	}
	logger.Debug("a.getDefaultSourceName() ", a.getDefaultSourceName())
	if a.getDefaultSourceName() != source.Name {
		if strings.Contains(source.Name, "platform-rk809-sound.analog-stereo") {
			a.ctx.SetDefaultSource("record_mono")
			a.updateDefaultSource(source.Name)
			logger.Debugf("set default source %s as record_mono", source.Name)
		} else {
			a.ctx.SetDefaultSource(source.Name)
		}
	}

	return nil
}

// SetPort activate the port for the special card.
// The available sinks and sources will also change with the profile changing.
func (a *Audio) SetPort(cardId uint32, portName string, direction int32) *dbus.Error {
	logger.Infof("dbus call SetPort with cardId %d, portName %s and direction %d", cardId, portName, direction)

	if !a.isPortEnabled(cardId, portName, direction) {
		return dbusutil.ToError(fmt.Errorf("card idx: %d, port name: %q is disabled", cardId, portName))
	}

	err := a.setPort(cardId, portName, int(direction))
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	card, err := a.cards.get(cardId)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	if int(direction) == pulse.DirectionSink {
		logger.Debugf("output port %s %s now is first priority", card.core.Name, portName)

		// TODO: 静音逻辑变更为端口切换时静音状态不变
		// sink := a.getDefaultSink()
		// if sink == nil {
		// 	return dbusutil.ToError(fmt.Errorf("can not get default sink"))
		// }
		// sink.setMute(false)

		// TODO: 删除
		// priorities.SetOutputPortFirst(card.core.Name, portName)
		// err = priorities.Save(globalPrioritiesFilePath)
		// priorities.Print()
		GetPriorityManager().SetFirstOutputPort(card.core.Name, portName)
	} else {
		logger.Debugf("input port %s %s now is first priority", card.core.Name, portName)

		// TODO: 静音逻辑变更为端口切换时静音状态不变
		// source := a.getDefaultSource()
		// if source == nil {
		// 	return dbusutil.ToError(fmt.Errorf("can not get default source"))
		// }
		// source.setMute(false)

		// TODO: 删除
		// priorities.SetInputPortFirst(card.core.Name, portName)
		// err = priorities.Save(globalPrioritiesFilePath)
		// priorities.Print()
		GetPriorityManager().SetFirstInputPort(card.core.Name, portName)
	}

	return dbusutil.ToError(err)
}

func (a *Audio) findSinks(cardId uint32, activePortName string) []*Sink {
	sinks := make([]*Sink, 0)
	for _, sink := range a.sinks {
		if sink.Card == cardId && sink.ActivePort.Name == activePortName {
			sinks = append(sinks, sink)
		}
	}

	return sinks
}

func (a *Audio) findSources(cardId uint32, activePortName string) []*Source {
	sources := make([]*Source, 0)
	for _, source := range a.sources {
		if source.Card == cardId && source.ActivePort.Name == activePortName {
			sources = append(sources, source)
		}
	}

	return sources
}

func (a *Audio) SetPortEnabled(cardId uint32, portName string, enabled bool) *dbus.Error {
	logger.Infof("dbus call SetPortEnabled with cardId %d, portName %s and enabled %t", cardId, portName, enabled)

	if !a.enableAutoSwitchPort {
		err := errors.New("DConfig of org.deepin.dde.daemon.audio autoSwitchPort is false")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	GetConfigKeeper().SetEnabled(a.getCardNameById(cardId), portName, enabled)

	err := a.service.Emit(a, "PortEnabledChanged", cardId, portName, enabled)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	a.setPropCards(a.cards.string())
	a.setPropCardsWithoutUnavailable(a.cards.stringWithoutUnavailable())
	GetPriorityManager().SetPorts(a.cards)
	a.autoSwitchPort()

	sinks := a.findSinks(cardId, portName)
	for _, sink := range sinks {
		sink.setMute(!enabled || GetConfigKeeper().Mute.MuteOutput)
	}

	sources := a.findSources(cardId, portName)
	for _, source := range sources {
		source.setMute(!enabled || GetConfigKeeper().Mute.MuteInput)
	}

	return nil
}

func (a *Audio) IsPortEnabled(cardId uint32, portName string) (enabled bool, busErr *dbus.Error) {
	// 不建议使用这个接口，可以从Cards和CardsWithoutUnavailable属性中获取此状态
	logger.Infof("dbus call IsPortEnabled with cardId %d and portName %s", cardId, portName)

	_, portConfig := GetConfigKeeper().GetCardAndPortConfig(a.getCardNameById(cardId), portName)
	return portConfig.Enabled, nil
}

func (a *Audio) setPort(cardId uint32, portName string, direction int) error {
	logger.Debugf("set port %d %s", cardId, portName)
	if a.ReduceNoise {
		// 切端口时要关闭降噪，但是设置属性会触发回调
		// 导致此关闭写入到配置文件中
		// 由于进行了端口切换，此时写配置文件时写的是新端口的配置
		// 属性交给配置恢复的流程处理改写
		a.setReduceNoise(false)
		a.ReduceNoise = false
		a.emitPropChangedReduceNoise(a.ReduceNoise)
	}
	a.portLocker.Lock()
	defer a.portLocker.Unlock()
	var (
		oppositePort      string
		oppositeDirection int
	)
	switch direction {
	case pulse.DirectionSink:
		oppositePort = a.getDefaultSourceActivePortName()
		oppositeDirection = pulse.DirectionSource
	case pulse.DirectionSource:
		oppositePort = a.getDefaultSinkActivePortName()
		oppositeDirection = pulse.DirectionSink
	default:
		return fmt.Errorf("invalid port direction: %d", direction)
	}

	a.mu.Lock()
	card, _ := a.cards.get(cardId)
	a.mu.Unlock()
	if card == nil {
		return fmt.Errorf("not found card #%d", cardId)
	}

	var err error
	targetPortInfo, err := card.Ports.Get(portName, direction)
	if err != nil {
		return err
	}

	setDefaultPort := func() error {
		if int(direction) == pulse.DirectionSink {
			return a.setDefaultSinkWithPort(cardId, portName)
		}
		return a.setDefaultSourceWithPort(cardId, portName)
	}

	// 蓝牙特殊情况下会出错，导致profile为off, 需要重新寻找合适的
	if targetPortInfo.Profiles != nil && targetPortInfo.Profiles.Exists(card.ActiveProfile.Name) && card.ActiveProfile.Name != "off" {
		// no need to change profile
		return setDefaultPort()
	}
	if portName == dndVirtualSinkName || cardId == math.MaxUint32 {
		return setDefaultPort()
	}

	// match the common profile contain sinkPort and sourcePort
	oppositePortInfo, _ := card.Ports.Get(oppositePort, oppositeDirection)
	commonProfiles := getCommonProfiles(targetPortInfo, oppositePortInfo)
	var targetProfile string
	if len(commonProfiles) != 0 {
		targetProfile = commonProfiles[0].Name
	} else {
		name, err := card.tryGetProfileByPort(portName)
		if err != nil {
			return err
		}
		targetProfile = name
	}
	// workaround for bluetooth, set profile to 'a2dp_sink' when port direction is output
	if direction == pulse.DirectionSink && targetPortInfo.Profiles.Exists("a2dp_sink") {
		targetProfile = "a2dp_sink"
	}
	card.core.SetProfile(targetProfile)
	logger.Debug("set profile", targetProfile)
	return setDefaultPort()
}

func (a *Audio) resetSinksVolume() {
	logger.Debug("reset sink volume", defaultOutputVolume)
	for _, s := range a.ctx.GetSinkList() {
		a.ctx.SetSinkMuteByIndex(s.Index, false)
		curPort := s.ActivePort.Name
		portList := s.Ports
		sidx := s.Index
		for _, port := range portList {
			a.ctx.SetSinkPortByIndex(sidx, port.Name)
			// wait port active
			time.Sleep(time.Millisecond * 100)
			s, _ = a.ctx.GetSink(sidx)
			pname := strings.ToLower(port.Name)
			var cv pulse.CVolume
			if strings.Contains(pname, "headphone") || strings.Contains(pname, "headset") {
				cv = s.Volume.SetAvg(defaultHeadphoneOutputVolume).SetBalance(s.ChannelMap,
					0).SetFade(s.ChannelMap, 0)
			} else {
				cv = s.Volume.SetAvg(defaultOutputVolume).SetBalance(s.ChannelMap,
					0).SetFade(s.ChannelMap, 0)
			}
			a.ctx.SetSinkVolumeByIndex(sidx, cv)
			time.Sleep(time.Millisecond * 100)
		}
		a.ctx.SetSinkPortByIndex(sidx, curPort)
	}
}

func (a *Audio) resetSourceVolume() {
	logger.Debug("reset source volume", defaultInputVolume)
	for _, s := range a.ctx.GetSourceList() {
		if s.ActivePort.Name != "" {
			a.ctx.SetSourceMuteByIndex(s.Index, false)
			cv := s.Volume.SetAvg(defaultInputVolume).SetBalance(s.ChannelMap,
				0).SetFade(s.ChannelMap, 0)
			a.ctx.SetSourceVolumeByIndex(s.Index, cv)
		}
	}
}

func (a *Audio) Reset() *dbus.Error {
	logger.Infof("dbus call Reset")

	a.resetSinksVolume()
	a.resetSourceVolume()
	gsSoundEffect := gio.NewSettings(gsSchemaSoundEffect)
	gsSoundEffect.Reset(gsKeyEnabled)
	gsSoundEffect.Unref()
	return nil
}

func (a *Audio) moveSinkInputsToSink(sinkId uint32) {
	a.mu.Lock()
	if len(a.sinkInputs) == 0 {
		a.mu.Unlock()
		return
	}
	var list []uint32
	for _, sinkInput := range a.sinkInputs {
		if sinkInput.getPropSinkIndex() == sinkId {
			continue
		}

		list = append(list, sinkInput.index)
	}
	a.mu.Unlock()
	if len(list) == 0 {
		return
	}
	logger.Debugf("move sink inputs %v to sink #%d", list, sinkId)
	a.ctx.MoveSinkInputsByIndex(list, sinkId)
}

func isPortExists(name string, ports []pulse.PortInfo) bool {
	for _, port := range ports {
		if port.Name == name {
			return true
		}
	}
	return false
}

func (*Audio) GetInterfaceName() string {
	return dbusInterface
}

func (a *Audio) resumeSinkConfig(s *Sink) {
	if s == nil {
		logger.Warning("nil sink")
		return
	}
	if s.ActivePort.Name == "" {
		logger.Debug("no active port")
		return
	}

	logger.Debugf("resume sink %s %s", a.getCardNameById(s.Card), s.ActivePort.Name)
	_, portConfig := GetConfigKeeper().GetCardAndPortConfig(a.getCardNameById(s.Card), s.ActivePort.Name)

	a.IncreaseVolume.Set(portConfig.IncreaseVolume)
	if portConfig.IncreaseVolume {
		a.MaxUIVolume = increaseMaxVolume
	} else {
		a.MaxUIVolume = normalMaxVolume
	}

	err := s.setVBF(portConfig.Volume, portConfig.Balance, 0.0)
	if err != nil {
		logger.Warning(err)
	}

	s.setMute(GetConfigKeeper().Mute.MuteOutput)

	if !portConfig.Enabled {
		// 意外原因切换到被禁用的端口上，例如没有可用端口
		s.setMute(true)
	}
}

func (a *Audio) resumeSourceConfig(s *Source, isPhyDev bool) {
	if s == nil {
		logger.Warning("nil source")
		return
	}
	if s.ActivePort.Name == "" {
		logger.Debug("no active port")
		return
	}

	logger.Debugf("resume source %s %s", a.getCardNameById(s.Card), s.ActivePort.Name)
	_, portConfig := GetConfigKeeper().GetCardAndPortConfig(a.getCardNameById(s.Card), s.ActivePort.Name)

	err := s.setVBF(portConfig.Volume, portConfig.Balance, 0.0)
	if err != nil {
		logger.Warning(err)
	}

	s.setMute(GetConfigKeeper().Mute.MuteInput)

	// 蓝牙不支持噪音抑制
	if portConfig.ReduceNoise && isBluezAudio(s.Name) {
		logger.Debug("bluetooth audio device cannot open reduce-noise")
		GetConfigKeeper().SetReduceNoise(a.getCardNameById(s.Card), s.ActivePort.Name, false)
	}

	// 不要在降噪通道上重复开启降噪
	if isPhyDev {
		logger.Debugf("physical source, set reduce noise %v", portConfig.ReduceNoise)
		err := a.setReduceNoise(portConfig.ReduceNoise)
		if err != nil {
			logger.Warning(err)
		} else {
			a.ReduceNoise = portConfig.ReduceNoise
			a.emitPropChangedReduceNoise(a.ReduceNoise)
		}

	} else {
		logger.Debugf("reduce noise source, set reduce noise %v", portConfig.ReduceNoise)
		a.ReduceNoise = portConfig.ReduceNoise
		a.emitPropChangedReduceNoise(a.ReduceNoise)
	}

	if !portConfig.Enabled {
		// 意外原因切换到被禁用的端口上，例如没有可用端口
		s.setMute(true)
	}
}

func (a *Audio) refreshBluetoothOpts() {
	if a.defaultSink == nil {
		return
	}
	// 云平台无card
	if a.defaultSink.Card == math.MaxUint32 {
		return
	}
	card, err := a.cards.get(a.defaultSink.Card)
	if err != nil {
		logger.Warning(err)
		return
	}

	a.setPropBluetoothAudioModeOpts(card.BluezModeOpts())
	a.setPropBluetoothAudioMode(card.BluezMode())
}

func (a *Audio) updateDefaultSink(sinkName string) {
	sinkInfo := a.getSinkInfoByName(sinkName)

	if sinkInfo == nil {
		logger.Warning("failed to get sinkInfo for name:", sinkName)
		a.setPropDefaultSink("/")
		return
	}
	logger.Debugf("updateDefaultSink #%d %s", sinkInfo.Index, sinkName)
	a.moveSinkInputsToSink(sinkInfo.Index)
	if !isPhysicalDevice(sinkName) {
		sinkInfo = a.getSinkInfoByName(sinkInfo.PropList["device.master_device"])
		if sinkInfo == nil {
			logger.Warning("failed to get virtual device sinkInfo for name:", sinkName)
			return
		}
	}
	a.mu.Lock()
	sink, ok := a.sinks[sinkInfo.Index]
	a.mu.Unlock()
	if !ok {
		// a.sinks 是缓存的 sink 信息，未查到 sink 信息，需要重新通过 pulseaudio 查询 sink 信息
		logger.Warningf("update sink %d", sinkInfo.Index)
		sink = a.updateSinks(sinkInfo.Index)
		logger.Debugf("updated sink %d", sinkInfo.Index)
		if sink == nil {
			logger.Warningf("not found sink #%d", sinkInfo.Index)
			a.setPropDefaultSink("/")
			return
		}
	}

	a.defaultSink = sink
	defaultSinkPath := sink.getPath()

	a.resumeSinkConfig(sink)

	a.PropsMu.Lock()
	a.setPropDefaultSink(defaultSinkPath)
	a.PropsMu.Unlock()

	logger.Debug("set prop default sink:", defaultSinkPath)
}

func (a *Audio) updateSources(index uint32) (source *Source) {
	sourceInfoList := a.ctx.GetSourceList()
	for _, sourceInfo := range sourceInfoList {
		// 如果音频为输入，过滤到所有的monitor
		if strings.HasSuffix(sourceInfo.Name, ".monitor") {
			logger.Debugf("skip %s source update", sourceInfo.Name)
			continue
		}
		// 判断 pulseaudio 的 source 索引是否存在，并返回存在的 source 信息
		if sourceInfo.Index == index {
			logger.Debug("get same source index:", index)
			source := newSource(sourceInfo, a)
			a.sources[index] = source
			sourcePath := source.getPath()
			err := a.service.Export(sourcePath, source)
			if err != nil {
				logger.Warning(err)
			}
			return source
		}
	}
	return nil
}

func (a *Audio) updateSinks(index uint32) (sink *Sink) {
	sinkInfoList := a.ctx.GetSinkList()
	for _, sinkInfo := range sinkInfoList {
		// 判断pulseaudio的sink索引是否存在，并返回存在的sink信息
		if sinkInfo.Index == index {
			logger.Debug("get same sink index:", index)
			sink := newSink(sinkInfo, a)
			logger.Debug("done")
			a.mu.Lock()
			a.sinks[index] = sink
			a.mu.Unlock()
			sinkPath := sink.getPath()
			err := a.service.Export(sinkPath, sink)
			if err != nil {
				logger.Warning(err)
			}
			return sink
		}
	}
	return nil
}

func (a *Audio) updateDefaultSource(sourceName string) {
	sourceInfo := a.getSourceInfoByName(sourceName)
	if sourceInfo == nil {
		logger.Warning("failed to get sourceInfo for name:", sourceName)
		a.setPropDefaultSource("/")
		return
	}
	logger.Debugf("updateDefaultSource #%d %s", sourceInfo.Index, sourceName)

	if !isPhysicalDevice(sourceName) {
		sourceInfo = a.getSourceInfoByName(sourceInfo.Proplist["device.master_device"])
		if sourceInfo == nil {
			logger.Warning("failed to get virtual device sourceInfo for name:", sourceName)
			return
		}
	}
	logger.Debug("sourceInfo", sourceInfo)
	a.mu.Lock()
	source, ok := a.sources[sourceInfo.Index]
	a.mu.Unlock()
	if !ok {
		// a.sources 是缓存的 source 信息，未查到 source 信息，需要重新通过 pulseaudio 查询 source 信息
		source = a.updateSources(sourceInfo.Index)
		if source == nil {
			logger.Warningf("not found source #%d", sourceInfo.Index)
			a.setPropDefaultSource("/")
			return
		}
	}

	if strings.Contains(source.Name, "record_mono") {
		sourceInfo := a.getSourceInfoByName("alsa_input.platform-rk809-sound.analog-stereo")
		if sourceInfo != nil {
			a.mu.Lock()
			if v, ok := a.sources[sourceInfo.Index]; ok {
				source = v
			}
			a.mu.Unlock()
		}
	}
	a.mu.Lock()
	a.defaultSource = source
	a.mu.Unlock()

	defaultSourcePath := source.getPath()

	a.resumeSourceConfig(source, isPhysicalDevice(sourceName))

	a.PropsMu.Lock()
	a.setPropDefaultSource(defaultSourcePath)
	a.PropsMu.Unlock()

	logger.Debug("set prop default source:", defaultSourcePath)
}

func (a *Audio) context() *pulse.Context {
	a.mu.Lock()
	c := a.ctx
	a.mu.Unlock()
	return c
}

func (a *Audio) moveSinkInputsToDefaultSink() {
	a.mu.Lock()
	if a.defaultSink == nil {
		a.mu.Unlock()
		return
	}
	defaultSinkIndex := a.defaultSink.index
	a.mu.Unlock()
	a.moveSinkInputsToSink(defaultSinkIndex)
}

func (a *Audio) getDefaultSource() *Source {
	a.mu.Lock()
	v := a.defaultSource
	a.mu.Unlock()
	return v
}

func (a *Audio) getDefaultSourceName() string {
	source := a.getDefaultSource()
	if source == nil {
		return ""
	}

	source.PropsMu.RLock()
	v := source.Name
	source.PropsMu.RUnlock()
	return v
}

func (a *Audio) getDefaultSink() *Sink {
	a.mu.Lock()
	v := a.defaultSink
	a.mu.Unlock()
	return v
}

func (a *Audio) getDefaultSinkName() string {
	sink := a.getDefaultSink()
	if sink == nil {
		return ""
	}

	sink.PropsMu.RLock()
	v := sink.Name
	sink.PropsMu.RUnlock()
	return v
}

func (a *Audio) getSinkInfoByName(sinkName string) *pulse.Sink {
	for _, sinkInfo := range a.ctx.GetSinkList() {
		if sinkInfo.Name == sinkName {
			return sinkInfo
		}
	}
	return nil
}

func (a *Audio) getSourceInfoByName(sourceName string) *pulse.Source {
	for _, sourceInfo := range a.ctx.GetSourceList() {
		if sourceInfo.Name == sourceName {
			return sourceInfo
		}
	}
	return nil
}
func getBestPort(ports []pulse.PortInfo) pulse.PortInfo {
	var portUnknown pulse.PortInfo
	var portYes pulse.PortInfo
	for _, port := range ports {
		if port.Available == pulse.AvailableTypeYes {
			if port.Priority > portYes.Priority || portYes.Name == "" {
				portYes = port
			}
		} else if port.Available == pulse.AvailableTypeUnknow {
			if port.Priority > portUnknown.Priority || portUnknown.Name == "" {
				portUnknown = port
			}
		}
	}

	if portYes.Name != "" {
		return portYes
	}
	return portUnknown
}

func (a *Audio) fixActivePortNotAvailable() {
	sinkInfoList := a.ctx.GetSinkList()
	for _, sinkInfo := range sinkInfoList {
		activePort := sinkInfo.ActivePort

		if activePort.Available == pulse.AvailableTypeNo {
			newPort := getBestPort(sinkInfo.Ports)
			if newPort.Name != activePort.Name && newPort.Name != "" {
				logger.Info("auto switch to port", newPort.Name)
				a.ctx.SetSinkPortByIndex(sinkInfo.Index, newPort.Name)
				a.saveConfig()
			}
		}
	}
}

func (a *Audio) NoRestartPulseAudio() *dbus.Error {
	a.noRestartPulseAudio = true
	return nil
}

// 当蓝牙声卡配置文件选择a2dp时,不支持声音输入,所以需要禁用掉,否则会录入
func (a *Audio) disableBluezSourceIfProfileIsA2dp() {
	a.mu.Lock()
	source, ok := a.sources[a.sourceIdx]
	if !ok {
		a.mu.Unlock()
		return
	}
	delete(a.sources, a.sourceIdx)
	a.mu.Unlock()
	a.updatePropSources()

	err := a.service.StopExport(source)
	if err != nil {
		logger.Warning(err)
		return
	}
}

func (a *Audio) isPortEnabled(cardId uint32, portName string, direction int32) bool {
	// 判断cardId 以及 portName的有效性
	a.mu.Lock()
	card, _ := a.cards.get(cardId)
	a.mu.Unlock()
	if card == nil {
		logger.Warningf("not found card #%d", cardId)
		return false
	}

	var err error
	_, err = card.Ports.Get(portName, int(direction))
	if err != nil {
		logger.Warningf("get port %s info failed: %v", portName, err)
		return false
	}

	_, portConfig := GetConfigKeeper().GetCardAndPortConfig(a.getCardNameById(cardId), portName)
	return portConfig.Enabled
}

// 设置蓝牙模式
func (a *Audio) SetBluetoothAudioMode(mode string) *dbus.Error {
	logger.Infof("dbus call SetBluetoothAudioMode with mode %s", mode)

	card, err := a.cards.get(a.defaultSink.Card)
	if err != nil {
		logger.Warning(err)
	}

	if !isBluezAudio(card.core.Name) {
		err = fmt.Errorf("current card %s is not bluetooth audio device", card.core.Name)
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	for _, profile := range card.Profiles {
		/* 这里需要注意，profile.Available为0表示不可用，非0表示未知 */
		logger.Debugf("check profile %s contains %s is %v && available != no is %v",
			profile.Name, mode, strings.Contains(strings.ToLower(profile.Name), mode),
			profile.Available != 0)
		if strings.Contains(strings.ToLower(profile.Name), mode) &&
			profile.Available != 0 {

			GetBluezAudioManager().SetMode(card.core.Name, mode)
			logger.Debugf("set profile %s", profile.Name)
			card.core.SetProfile(profile.Name)

			// 手动切换蓝牙模式为headset，
			if mode == bluezModeHeadset || mode == bluezModeHandsfree {
				a.inputAutoSwitchCount = 0
				GetPriorityManager().Input.SetTheFirstType(PortTypeBluetooth)
			}
			return nil
		}
	}

	return dbusutil.ToError(fmt.Errorf("%s cannot support %s mode", card.core.Name, mode))
}

func (a *Audio) setEnableAutoSwitchPort(value bool) {
	a.PropsMu.Lock()
	a.enableAutoSwitchPort = value
	a.PropsMu.Unlock()

	if a.controlCenterGsSettings == nil {
		return
	} else if !isStringInSlice(a.controlCenterGsSettings.ListKeys(), gsKeyDeviceManager) {
		return
	}

	if a.enableAutoSwitchPort != a.controlCenterDeviceManager.Get() {
		a.controlCenterDeviceManager.Set(a.enableAutoSwitchPort)
		logger.Info("setEnableAutoSwitchPort set a.enableAutoSwitchPort to a.controlCenterDeviceManager. value : ", a.enableAutoSwitchPort)
	}
}

// 初始化 dsg 配置的属性
func (a *Audio) initDsgProp() error {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}

	a.systemSigLoop = dbusutil.NewSignalLoop(systemBus, 10)

	systemConnObj := systemBus.Object("org.desktopspec.ConfigManager", "/")
	err = systemConnObj.Call("org.desktopspec.ConfigManager.acquireManager", 0, "org.deepin.dde.daemon", "org.deepin.dde.daemon.audio", "").Store(&a.configManagerPath)
	if err != nil {
		logger.Warning(err)
		return nil
	}

	err = dbusutil.NewMatchRuleBuilder().Type("signal").
		PathNamespace(string(a.configManagerPath)).
		Interface("org.desktopspec.ConfigManager.Manager").
		Member("valueChanged").Build().AddTo(systemBus)
	if err != nil {
		logger.Warning(err)
		return nil
	}

	var val bool
	systemConnObj = systemBus.Object("org.desktopspec.ConfigManager", a.configManagerPath)
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, dsgKeyAutoSwitchPort).Store(&val)
	if err != nil {
		logger.Warning(err)
	} else {
		logger.Info("auto switch port:", val)
		a.setEnableAutoSwitchPort(val)
	}

	var keyPausePlayer bool
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, dsgkeyPausePlayer).Store(&keyPausePlayer)
	if err != nil {
		logger.Warning(err)
	} else {
		logger.Info("auto switch port:", keyPausePlayer)
		a.PropsMu.Lock()
		a.PausePlayer = keyPausePlayer
		a.PropsMu.Unlock()
	}

	var ret []dbus.Variant
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, dsgKeyBluezModeFilterList).Store(&ret)
	if err != nil {
		logger.Warning(err)
	} else {
		bluezModeFilterList = bluezModeFilterList[:0]
		for i := range ret {
			if v, ok := ret[i].Value().(string); ok {
				bluezModeFilterList = append(bluezModeFilterList, v)
			}
		}
		logger.Info("bluez filter audio mode opts", bluezModeFilterList)
	}

	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, dsgKeyPortFilterList).Store(&ret)
	if err != nil {
		logger.Warning(err)
	} else {
		portFilterList = portFilterList[:0]
		for i := range ret {
			if v, ok := ret[i].Value().(string); ok {
				portFilterList = append(portFilterList, v)
			}
		}
		logger.Info("port filter list", portFilterList)
	}

	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, dsgKeyReduceNoise).Store(&defaultReduceNoise)
	if err != nil {
		logger.Warning(err)
	} else {
		logger.Info("default reduce noise status:", defaultReduceNoise)
	}

	ret = make([]dbus.Variant, 0)
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, dsgKeyInputDefaultPriorities).Store(&ret)
	if err != nil {
		logger.Warning(err)
	} else {
		for i := range ret {
			if v, ok := ret[i].Value().(float64); ok {
				inputDefaultPriorities = append(inputDefaultPriorities, int(v))
			}
		}
		logger.Info("input default priority list", inputDefaultPriorities)
	}

	ret = make([]dbus.Variant, 0)
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, dsgKeyOutputDefaultPriorities).Store(&ret)
	if err != nil {
		logger.Warning(err)
	} else {
		for i := range ret {
			if v, ok := ret[i].Value().(float64); ok {
				outputDefaultPriorities = append(outputDefaultPriorities, int(v))
			}
		}
		logger.Info("output default priority list", outputDefaultPriorities)
	}

	getBluezModeDefault := func() {
		var val string
		err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, dsgKeyBluezModeDefault).Store(&val)
		if err != nil {
			logger.Warning(err)
		} else {
			if val == bluezModeA2dp || val == bluezModeHandsfree || val == bluezModeHeadset {
				bluezModeDefault = val
				logger.Info("bluez default mode:", bluezModeDefault)
			}
		}
	}
	getBluezModeDefault()

	// 监听dsg配置变化
	a.systemSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.desktopspec.ConfigManager.Manager.valueChanged",
	}, func(sig *dbus.Signal) {
		if strings.Contains(sig.Name, "org.desktopspec.ConfigManager.Manager.valueChanged") &&
			strings.Contains(string(sig.Path), "org_deepin_dde_daemon_audio") && len(sig.Body) >= 1 {
			key, ok := sig.Body[0].(string)
			if ok {
				switch key {
				case dsgKeyAutoSwitchPort:
					var val bool
					err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, key).Store(&val)
					if err != nil {
						logger.Warning(err)
					} else {
						logger.Info("auto switch port:", val)
						a.setEnableAutoSwitchPort(val)
					}
				case dsgKeyBluezModeDefault:
					getBluezModeDefault()
				}
			}

			if ok && key == dsgkeyPausePlayer {
				var pausePlayer bool
				err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, key).Store(&pausePlayer)
				if err != nil {
					logger.Warning(err)
				} else {
					logger.Info("pausePlayer config:", pausePlayer)
					a.PropsMu.Lock()
					a.PausePlayer = pausePlayer
					a.emitPropChangedPausePlayer(pausePlayer)
					a.PropsMu.Unlock()
				}
			}
		}
	})

	a.systemSigLoop.Start()

	return nil
}

// 是否支持自动端口切换策略
func (a *Audio) canAutoSwitchPort() bool {
	a.PropsMu.RLock()
	defer a.PropsMu.RUnlock()

	return a.enableAutoSwitchPort
}
