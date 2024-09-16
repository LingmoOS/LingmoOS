// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"encoding/json"
	"errors"
	"io/ioutil"
	"os"
	"sync"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-api/powersupply"
	"github.com/linuxdeepin/dde-api/powersupply/battery"
	ConfigManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	DisplayManager "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.DisplayManager"
	gudev "github.com/linuxdeepin/go-gir/gudev-1.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/strv"
)

const (
	dsettingsAppID                                = "org.deepin.dde.daemon"
	dsettingsPowerName                            = "org.deepin.dde.daemon.power"
	dsettingsPowerSavingModeEnabled               = "powerSavingModeEnabled"
	dsettingsPowerSavingModeAuto                  = "powerSavingModeAuto"
	dsettingsPowerSavingModeAutoWhenBatteryLow    = "powerSavingModeAutoWhenBatteryLow"
	dsettingsPowerSavingModeBrightnessDropPercent = "powerSavingModeBrightnessDropPercent"
	dsettingsPowerMappingConfig                   = "powerMappingConfig"
	dsettingsMode                                 = "mode"
)

type supportMode struct {
	Balance    bool `json:"balance"`
	Performace bool `json:"performace"`
	PowerSave  bool `json:"powersave"`
}

//go:generate dbusutil-gen -type Manager,Battery -import github.com/linuxdeepin/dde-api/powersupply/battery manager.go battery.go
//go:generate dbusutil-gen em -type Manager,Battery

// https://www.kernel.org/doc/Documentation/power/power_supply_class.txt
type Manager struct {
	service       *dbusutil.Service
	systemSigLoop *dbusutil.SignalLoop
	batteries     map[string]*Battery
	batteriesMu   sync.Mutex
	ac            *AC
	gudevClient   *gudev.Client
	dsgPower      ConfigManager.Manager
	// 电池是否电量低
	batteryLow bool
	// 初始化是否完成
	initDone bool

	// INFO: pstate中，可选项是performance balance_performance balance_power power

	PropsMu      sync.RWMutex
	OnBattery    bool
	HasLidSwitch bool
	// battery display properties:
	HasBattery         bool
	BatteryPercentage  float64
	BatteryStatus      battery.Status
	BatteryTimeToEmpty uint64
	BatteryTimeToFull  uint64
	// 电池容量
	BatteryCapacity float64

	// 开启和关闭节能模式
	PowerSavingModeEnabled bool `prop:"access:rw"`

	// 自动切换节能模式，依据为是否插拔电源
	PowerSavingModeAuto bool `prop:"access:rw"`

	// 低电量时自动开启
	PowerSavingModeAutoWhenBatteryLow bool `prop:"access:rw"`

	// 开启节能模式时降低亮度的百分比值
	PowerSavingModeBrightnessDropPercent uint32 `prop:"access:rw"`

	// 开启节能模式时保存的数据
	PowerSavingModeBrightnessData string `prop:"access:rw"`

	// CPU频率调节模式，支持powersave和performance
	CpuGovernor string

	// CPU频率增强是否开启
	CpuBoost bool

	// 是否支持Boost
	IsHighPerformanceSupported bool

	// 是否支持平衡模式
	IsBalanceSupported bool

	// 是否支持节能模式
	IsPowerSaveSupported bool

	// 是否支持切换性能模式
	SupportSwitchPowerMode bool `prop:"access:rw"`
	// 当前模式
	Mode string

	// 是否在启动阶段，启动阶段不允许调节亮度; 若在启动阶段切换模式后(切节能模式降低亮度)，可以调节亮度.
	IsInBootTime bool

	// 上次非低电量时的模式
	lastMode string

	displayManager DisplayManager.DisplayManager

	isLowBatteryMode bool
	// nolint
	signals *struct {
		BatteryDisplayUpdate struct {
			timestamp int64
		}

		BatteryAdded struct {
			path dbus.ObjectPath
		}

		BatteryRemoved struct {
			path dbus.ObjectPath
		}

		LidClosed struct{}
		LidOpened struct{}
	}
}

const (
	ddePowerSave   = "powersave"
	ddeBalance     = "balance"
	ddePerformance = "performance"
	ddeLowBattery  = "lowBattery" // 内部使用，在对外暴露的时候，会切换成powersave
)

var _allPowerModeArray = []string{
	ddePowerSave,
	ddeBalance,
	ddePerformance,
	ddeLowBattery,
}

var _validPowerModeArray = strv.Strv{
	ddePowerSave,
	ddeBalance,
	ddePerformance,
	ddeLowBattery,
}

func newManager(service *dbusutil.Service) (*Manager, error) {
	m := &Manager{
		service:                    service,
		BatteryPercentage:          100,
		lastMode:                   ddeBalance,
		IsHighPerformanceSupported: true,
		IsBalanceSupported:         true,
		IsPowerSaveSupported:       true,
		CpuBoost:                   true,
	}

	err := m.init()
	if err != nil {
		m.destroy()
		return nil, err
	}

	return m, nil
}

type AC struct {
	gudevClient *gudev.Client
	sysfsPath   string
}

func newAC(manager *Manager, device *gudev.Device) *AC {
	sysfsPath := device.GetSysfsPath()
	return &AC{
		gudevClient: manager.gudevClient,
		sysfsPath:   sysfsPath,
	}
}

func (ac *AC) newDevice() *gudev.Device {
	return ac.gudevClient.QueryBySysfsPath(ac.sysfsPath)
}

func (m *Manager) refreshAC(ac *gudev.Device) { // 拔插电源时候触发
	online := ac.GetPropertyAsBoolean("POWER_SUPPLY_ONLINE")
	logger.Debug("ac online:", online)
	onBattery := !online

	m.PropsMu.Lock()
	m.setPropOnBattery(onBattery)
	m.PropsMu.Unlock()
	// 根据OnBattery的状态,修改节能模式
	m.updatePowerMode(false) // refreshAC
}

func (m *Manager) initAC(devices []*gudev.Device) {
	var ac *gudev.Device
	for _, dev := range devices {
		if powersupply.IsMains(dev) {
			ac = dev
			break
		}
	}
	if ac != nil {
		m.refreshAC(ac)
		m.ac = newAC(m, ac)
	}
}

func (m *Manager) init() error {
	m.systemSigLoop = dbusutil.NewSignalLoop(m.service.Conn(), 10)
	m.systemSigLoop.Start()
	err := m.initDsgConfig()
	if err != nil {
		logger.Warning(err)
	}

	subsystems := []string{"power_supply", "input"}
	m.gudevClient = gudev.NewClient(subsystems)
	if m.gudevClient == nil {
		return errors.New("gudevClient is nil")
	}

	m.initLidSwitch()
	devices := powersupply.GetDevices(m.gudevClient)

	m.initAC(devices)
	m.initBatteries(devices)
	for _, dev := range devices {
		dev.Unref()
	}

	m.gudevClient.Connect("uevent", m.handleUEvent)
	m.initDone = true

	m.updatePowerMode(true) // init

	m.displayManager = DisplayManager.NewDisplayManager(m.service.Conn())
	m.displayManager.InitSignalExt(m.systemSigLoop, true)
	return nil
}

func (m *Manager) initDsgConfig() error {
	logger.Info("org.deepin.dde.Power1 module start init dconfig.")
	// dsg 配置
	ds := ConfigManager.NewConfigManager(m.service.Conn())

	dsPowerPath, err := ds.AcquireManager(0, dsettingsAppID, dsettingsPowerName, "")
	if err != nil {
		return err
	}
	dsPower, err := ConfigManager.NewManager(m.service.Conn(), dsPowerPath)
	if err != nil {
		return err
	}
	m.dsgPower = dsPower

	cfg := loadConfigSafe()
	if cfg != nil {
		// 将config.json中的配置完成初始化
		m.PowerSavingModeEnabled = cfg.PowerSavingModeEnabled                             // 开启和关闭节能模式
		m.PowerSavingModeAuto = cfg.PowerSavingModeAuto                                   // 自动切换节能模式，依据为是否插拔电源
		m.PowerSavingModeAutoWhenBatteryLow = cfg.PowerSavingModeAutoWhenBatteryLow       // 低电量时自动开启
		m.PowerSavingModeBrightnessDropPercent = cfg.PowerSavingModeBrightnessDropPercent // 开启节能模式时降低亮度的百分比值
		m.Mode = cfg.Mode
		migrateErr := m.migrateFromCurrentConfigsToDsg()
		if migrateErr != nil {
			logger.Error("migrateFromCurrentConfigsToDsg failed, err:", migrateErr)
		}
	}

	getPowerSavingModeAuto := func(init bool) {
		data, err := dsPower.Value(0, dsettingsPowerSavingModeAuto)
		if err != nil {
			logger.Warning(err)
			return
		}

		if init {
			m.PowerSavingModeAuto = data.Value().(bool)
			return
		}

		m.setPropPowerSavingModeAuto(data.Value().(bool))
	}

	getPowerSavingModeEnabled := func(init bool) {
		data, err := dsPower.Value(0, dsettingsPowerSavingModeEnabled)
		if err != nil {
			logger.Warning(err)
			return
		}

		if init {
			m.PowerSavingModeEnabled = data.Value().(bool)
			return
		}

		m.setPropPowerSavingModeEnabled(data.Value().(bool))
	}

	getPowerSavingModeAutoWhenBatteryLow := func(init bool) {
		data, err := dsPower.Value(0, dsettingsPowerSavingModeAutoWhenBatteryLow)
		if err != nil {
			logger.Warning(err)
			return
		}

		if init {
			m.PowerSavingModeAutoWhenBatteryLow = data.Value().(bool)
			return
		}

		m.setPropPowerSavingModeAutoWhenBatteryLow(data.Value().(bool))
	}

	getPowerSavingModeBrightnessDropPercent := func(init bool) {
		data, err := dsPower.Value(0, dsettingsPowerSavingModeBrightnessDropPercent)
		if err != nil {
			logger.Warning(err)
			return
		}

		if init {
			switch vv := data.Value().(type) {
			case float64:
				m.PowerSavingModeBrightnessDropPercent = uint32(vv)
			case int64:
				m.PowerSavingModeBrightnessDropPercent = uint32(vv)
			default:
				logger.Warning("type is wrong! type : ", vv)
			}

			return
		}

		ret := false
		switch vv := data.Value().(type) {
		case float64:
			ret = m.setPropPowerSavingModeBrightnessDropPercent(uint32(vv))
		case int64:
			ret = m.setPropPowerSavingModeBrightnessDropPercent(uint32(vv))
		default:
			logger.Warning("type is wrong! type : ", vv)
		}
		if ret {
			logger.Info("Set power saving mode brightness drop percent", m.PowerSavingModeBrightnessDropPercent)
		}
	}

	getMode := func(init bool) string {
		ret, err := dsPower.Value(0, dsettingsMode)
		if err != nil {
			logger.Warning(err)
			return ddeBalance
		}

		value := ret.Value().(string)
		logger.Infof("value:%v", value)
		// dsg更新配置后，校验mode有效性
		if !_validPowerModeArray.Contains(value) {
			value = ddeBalance
			_ = m.setDsgData(dsettingsMode, value, m.dsgPower) // 将修正后的数据回写dconfig
		}
		if init {
			logger.Info("init ")
			m.Mode = value
			return value
		}
		m.setPropMode(value)
		return value
	}

	getPowerMappingConfig := func() {
		data, err := dsPower.Value(0, dsettingsPowerMappingConfig)
		if err != nil {
			logger.Warning(err)
			return
		}
		config := make(map[string]powerConfig)
		configStr := data.Value().(string)
		err = json.Unmarshal([]byte(configStr), &config)
		if err != nil {
			logger.Warning(err)
			return
		}

		for _, mode := range _allPowerModeArray {
			c, ok := config[mode]
			if ok {
				_powerConfigMap[mode].DSPCConfig = c.DSPCConfig
			}
		}
	}

	getPowerSavingModeAuto(true)
	getPowerSavingModeEnabled(true)
	getPowerSavingModeAutoWhenBatteryLow(true)
	getPowerSavingModeBrightnessDropPercent(true)
	getMode(true)
	getPowerMappingConfig()

	dsPower.InitSignalExt(m.systemSigLoop, true)
	_, _ = dsPower.ConnectValueChanged(func(key string) {
		logger.Info("dconfig org.deepin.dde.daemon.power valueChanged, key : ", key)
		switch key {
		case dsettingsPowerSavingModeAuto:
			getPowerSavingModeAuto(false)
		case dsettingsPowerSavingModeEnabled:
			getPowerSavingModeEnabled(false)
		case dsettingsPowerSavingModeAutoWhenBatteryLow:
			getPowerSavingModeAutoWhenBatteryLow(false)
		case dsettingsPowerSavingModeBrightnessDropPercent:
			getPowerSavingModeBrightnessDropPercent(false)
		case dsettingsMode:
			oldMode := m.Mode
			newMode := getMode(false)
			if oldMode == newMode {
				return
			}
			// 手动(外部请求)切换到节能模式，或节能模式切换到其他模式时，关闭电池自动节能和低电量自动节能
			if ddePowerSave == oldMode || ddePowerSave == newMode {
				m.PropsMu.Lock()
				m.updatePowerSavingState(false)
				m.PropsMu.Unlock()
			}
			m.doSetMode(newMode)
			return
		case dsettingsPowerMappingConfig:
			getPowerMappingConfig()
		default:
			logger.Debug("Not process. valueChanged, key : ", key)
		}
		m.updatePowerMode(false) // dconfig change
	})

	return nil
}

func (m *Manager) handleUEvent(client *gudev.Client, action string, device *gudev.Device) {
	logger.Debug("on uevent action:", action)
	defer device.Unref()

	switch action {
	case "change":
		if powersupply.IsMains(device) {
			if m.ac == nil {
				m.ac = newAC(m, device)
			} else if m.ac.sysfsPath != device.GetSysfsPath() {
				logger.Warning("found another AC", device.GetSysfsPath())
				return
			}

			// now m.ac != nil, and sysfsPath equal
			m.refreshAC(device)
			time.AfterFunc(1*time.Second, m.refreshBatteries)
			time.AfterFunc(3*time.Second, m.refreshBatteries)
			// 电源状态变更时，需要一段时间才能稳定，因此在1分钟内，每隔5秒刷新一次，保证数据及时更新
			for i := 1; i <= 12; i++ {
				time.AfterFunc(time.Duration(i*5)*time.Second, m.refreshBatteries)
			}
		} else if powersupply.IsSystemBattery(device) {
			m.addAndExportBattery(device)
		}
	case "add":
		if powersupply.IsSystemBattery(device) {
			m.addAndExportBattery(device)
		}
		// ignore add mains

	case "remove":
		if powersupply.IsSystemBattery(device) {
			m.removeBattery(device)
		}
	}

}

func (m *Manager) initBatteries(devices []*gudev.Device) {
	m.batteries = make(map[string]*Battery)
	for _, dev := range devices {
		m.addBattery(dev)
	}
	logger.Debugf("initBatteries done %#v", m.batteries)
}

func (m *Manager) addAndExportBattery(dev *gudev.Device) {
	bat, added := m.addBattery(dev)
	if added {
		err := m.service.Export(bat.getObjPath(), bat)
		if err == nil {
			m.emitBatteryAdded(bat)
		} else {
			logger.Warning("failed to export battery:", err)
		}
	}
}

func (m *Manager) addBattery(dev *gudev.Device) (*Battery, bool) {
	logger.Debug("addBattery dev:", dev)
	if !powersupply.IsSystemBattery(dev) {
		return nil, false
	}

	sysfsPath := dev.GetSysfsPath()
	logger.Debug(sysfsPath)

	m.batteriesMu.Lock()
	bat, ok := m.batteries[sysfsPath]
	m.batteriesMu.Unlock()
	if ok {
		logger.Debugf("add battery failed , sysfsPath exists %q", sysfsPath)
		bat.Refresh()
		return bat, false
	}

	bat = newBattery(m, dev)
	if bat == nil {
		logger.Debugf("add batteries failed, sysfsPath %q, new battery failed", sysfsPath)
		return nil, false
	}

	m.batteriesMu.Lock()
	m.batteries[sysfsPath] = bat
	m.refreshBatteryDisplay()
	m.batteriesMu.Unlock()
	bat.setRefreshDoneCallback(m.refreshBatteryDisplay)
	return bat, true
}

// removeBattery remove the battery from Manager.batteries, and stop export it.
func (m *Manager) removeBattery(dev *gudev.Device) {
	sysfsPath := dev.GetSysfsPath()

	m.batteriesMu.Lock()
	bat, ok := m.batteries[sysfsPath]
	m.batteriesMu.Unlock()

	if ok {
		logger.Info("removeBattery", sysfsPath)
		m.batteriesMu.Lock()
		delete(m.batteries, sysfsPath)
		m.refreshBatteryDisplay()
		m.batteriesMu.Unlock()

		err := m.service.StopExport(bat)
		if err != nil {
			logger.Warning(err)
		}
		m.emitBatteryRemoved(bat)

		bat.destroy()
	} else {
		logger.Warning("removeBattery failed: invalid sysfsPath ", sysfsPath)
	}
}

func (m *Manager) emitBatteryAdded(bat *Battery) {
	err := m.service.Emit(m, "BatteryAdded", bat.getObjPath())
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) emitBatteryRemoved(bat *Battery) {
	err := m.service.Emit(m, "BatteryRemoved", bat.getObjPath())
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) destroy() {
	logger.Debug("destroy")
	m.batteriesMu.Lock()
	for _, bat := range m.batteries {
		bat.destroy()
	}
	m.batteries = nil
	m.batteriesMu.Unlock()

	if m.gudevClient != nil {
		m.gudevClient.Unref()
		m.gudevClient = nil
	}
	m.systemSigLoop.Stop()
}

const configFile = "/var/lib/dde-daemon/power/config.json"

type Config struct {
	PowerSavingModeEnabled               bool
	PowerSavingModeAuto                  bool
	PowerSavingModeAutoWhenBatteryLow    bool
	PowerSavingModeBrightnessDropPercent uint32
	Mode                                 string
}

func loadConfig() (*Config, error) {
	content, err := ioutil.ReadFile(configFile)
	if err != nil {
		return nil, err
	}
	var cfg Config
	err = json.Unmarshal(content, &cfg)
	if err != nil {
		return nil, err
	}

	return &cfg, nil
}

func loadConfigSafe() *Config {
	cfg, err := loadConfig()
	if err != nil {
		// ignore not exist error
		if !os.IsNotExist(err) {
			logger.Warning(err)
		}
		return nil
	}
	// 新增字段后第一次启动时,缺少两个新增字段的json,导致亮度下降百分比字段默认为0,导致与默认值不符,需要处理
	// 低电量自动待机字段的默认值为false,不会导致错误影响
	// 正常情况下该字段范围为10-40,只有在该情况下会出现0的可能
	if cfg.PowerSavingModeBrightnessDropPercent == 0 {
		cfg.PowerSavingModeBrightnessDropPercent = 20
	}

	if cfg.Mode == "" {
		cfg.Mode = ddeBalance
	}
	return cfg
}

func (m *Manager) migrateFromCurrentConfigsToDsg() error {
	err := m.saveDsgConfig("")
	if err != nil {
		logger.Warning("saveDsgConfig failed", err)
		return err
	}

	// 迁移完成后，删除本地配置文件
	err = os.Remove(configFile)
	if err != nil {
		logger.Warning("delete local configs file failed", err)
		return err
	}

	return nil
}

func (m *Manager) saveDsgConfig(value string) (err error) {
	switch value {
	case "PowerSavingModeBrightnessDropPercent":
		err = m.setDsgData(dsettingsPowerSavingModeBrightnessDropPercent, int64(m.PowerSavingModeBrightnessDropPercent), m.dsgPower)
		if err != nil {
			return err
		}
	case "PowerSavingModeAutoWhenBatteryLow":
		err = m.setDsgData(dsettingsPowerSavingModeAutoWhenBatteryLow, m.PowerSavingModeAutoWhenBatteryLow, m.dsgPower)
		if err != nil {
			return err
		}
	case "PowerSavingModeEnabled":
		err = m.setDsgData(dsettingsPowerSavingModeEnabled, m.PowerSavingModeEnabled, m.dsgPower)
		if err != nil {
			return err
		}
	case "PowerSavingModeAuto":
		err = m.setDsgData(dsettingsPowerSavingModeAuto, m.PowerSavingModeAuto, m.dsgPower)
		if err != nil {
			return err
		}
	case "":
		err = m.setDsgData(dsettingsPowerSavingModeBrightnessDropPercent, int64(m.PowerSavingModeBrightnessDropPercent), m.dsgPower)
		if err != nil {
			return err
		}
		err = m.setDsgData(dsettingsPowerSavingModeAutoWhenBatteryLow, m.PowerSavingModeAutoWhenBatteryLow, m.dsgPower)
		if err != nil {
			return err
		}
		err = m.setDsgData(dsettingsPowerSavingModeEnabled, m.PowerSavingModeEnabled, m.dsgPower)
		if err != nil {
			return err
		}
		err = m.setDsgData(dsettingsPowerSavingModeAuto, m.PowerSavingModeAuto, m.dsgPower)
		if err != nil {
			return err
		}
	}

	return m.setDsgData(dsettingsMode, m.Mode, m.dsgPower)
}

func (m *Manager) doSetMode(mode string) {
	logger.Info(" doSetMode, mode : ", mode)
	if !_validPowerModeArray.Contains(mode) {
		logger.Errorf("PowerMode %q mode is not supported", mode)
		return
	}
	if mode == ddePowerSave && m.batteryLow {
		mode = ddeLowBattery
	}
	fixMode := mode
	if fixMode == ddeLowBattery {
		fixMode = ddePowerSave
		m.isLowBatteryMode = true
	} else {
		m.isLowBatteryMode = false
	}
	modeChanged := m.setPropMode(fixMode)
	if modeChanged {
		logger.Info("Set power mode", fixMode)
		m.IsInBootTime = false
	}

	// 处理ddeLowBattery情况，所以每次都要设置
	go m.setDSPCState(_powerConfigMap[mode].DSPCConfig) // doSetMode
	m.setPropPowerSavingModeEnabled(_powerConfigMap[mode].PowerSavingModeEnabled)

	if m.lastMode != mode && mode != ddePowerSave && mode != ddeLowBattery {
		m.lastMode = mode
	}
	if modeChanged {
		_ = m.setDsgData(dsettingsMode, fixMode, m.dsgPower)
	}
}

// 需求: 为了提高启动速度，登录前将性能模式设置为performance
// ① 为了减小耦合性，仅写文件(doSetCpuGovernor)，不修改后端相关属性
func (m *Manager) enablePerformanceInBoot() bool {
	if m.Mode == ddePerformance {
		return false
	}
	displaySessions, err := m.displayManager.Sessions().Get(0)
	if err != nil {
		logger.Warning(err)
	} else if len(displaySessions) > 0 {
		return false
	}
	go m.setDSPCState(DSPCPerformance)
	logger.Info("enablePerformanceInBoot performance")
	m.IsInBootTime = true
	return true
}
