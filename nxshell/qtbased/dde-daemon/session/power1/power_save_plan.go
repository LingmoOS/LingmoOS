// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"encoding/json"
	"errors"
	"io/ioutil"
	"math"
	"os"
	"strings"
	"sync"
	"time"

	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"

	"github.com/godbus/dbus/v5"
	ConfigManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gsettings"
	"github.com/linuxdeepin/go-lib/procfs"
	x "github.com/linuxdeepin/go-x11-client"
	xscreensaver "github.com/linuxdeepin/go-x11-client/ext/screensaver"
	"github.com/linuxdeepin/go-x11-client/util/wm/ewmh"
)

const submodulePSP = "PowerSavePlan"

const (
	dsettingsAppID                                     = "org.deepin.dde.daemon"
	dsettingsPowerName                                 = "org.deepin.dde.daemon.power"
	dsettingsAllowScreenSaver                          = "allowScreenSaver"
	dsettingsDelayWakeupInterval                       = "delayWakeupInterval"
	dsettingsDelayHandleIdleOffIntervalWhenScreenBlack = "delayHandleIdleOffIntervalWhenScreenBlack"
)

func init() {
	submoduleList = append(submoduleList, newPowerSavePlan)
}

type powerSavePlan struct {
	systemSigLoop      *dbusutil.SignalLoop
	dsgPower           ConfigManager.Manager
	manager            *Manager
	screenSaverTimeout int32
	metaTasks          metaTasks
	tasks              delayedTasks
	// key output name, value old brightness
	oldBrightnessTable map[string]float64
	mu                 sync.Mutex
	screensaverRunning bool

	atomNetWMStateFullscreen    x.Atom
	atomNetWMStateFocused       x.Atom
	fullscreenWorkaroundAppList []string

	brightnessSave         gsprop.String
	multiBrightnessWithPsm *multiBrightnessWithPsm
	psmEnabledTime         time.Time
	psmPercentChangedTime  time.Time
	modeBeforeIdle         string
	allowScreenSaver       bool
}

func newPowerSavePlan(manager *Manager) (string, submodule, error) {
	p := new(powerSavePlan)
	p.manager = manager

	conn := manager.helper.xConn
	var err error
	p.atomNetWMStateFullscreen, err = conn.GetAtom("_NET_WM_STATE_FULLSCREEN")
	if err != nil {
		return submodulePSP, nil, err
	}
	p.atomNetWMStateFocused, err = conn.GetAtom("_NET_WM_STATE_FOCUSED")
	if err != nil {
		return submodulePSP, nil, err
	}

	p.fullscreenWorkaroundAppList = manager.settings.GetStrv(
		"fullscreen-workaround-app-list")

	err = p.initDsgConfig()
	if err != nil {
		logger.Warning(err)
	}
	p.systemSigLoop.Start()

	return submodulePSP, p, nil
}

// 监听 GSettings 值改变, 更新节电计划
func (psp *powerSavePlan) initSettingsChangedHandler() {
	m := psp.manager
	gsettings.ConnectChanged(gsSchemaPower, "*", func(key string) {
		logger.Debug("setting changed", key)
		switch key {
		case settingKeyLinePowerScreensaverDelay,
			settingKeyLinePowerScreenBlackDelay,
			settingKeyLinePowerLockDelay,
			settingKeyLinePowerSleepDelay:
			if !m.OnBattery {
				logger.Debug("Change OnLinePower plan")
				psp.OnLinePower()
			}

		case settingKeyBatteryScreensaverDelay,
			settingKeyBatteryScreenBlackDelay,
			settingKeyBatteryLockDelay,
			settingKeyBatterySleepDelay:
			if m.OnBattery {
				logger.Debug("Change OnBattery plan")
				psp.OnBattery()
			}

		case settingKeyAmbientLightAdjuestBrightness:
			psp.manager.claimOrReleaseAmbientLight()
		}
	})
}

func (psp *powerSavePlan) OnBattery() {
	logger.Debug("Use OnBattery plan")
	m := psp.manager
	psp.Update(m.BatteryScreensaverDelay.Get(), m.BatteryLockDelay.Get(),
		m.BatteryScreenBlackDelay.Get(),
		m.BatterySleepDelay.Get())
}

func (psp *powerSavePlan) OnLinePower() {
	logger.Debug("Use OnLinePower plan")
	m := psp.manager
	psp.Update(m.LinePowerScreensaverDelay.Get(), m.LinePowerLockDelay.Get(),
		m.LinePowerScreenBlackDelay.Get(),
		m.LinePowerSleepDelay.Get())
}

func (psp *powerSavePlan) Reset() {
	m := psp.manager
	logger.Debug("OnBattery:", m.OnBattery)
	if m.OnBattery {
		psp.OnBattery()
	} else {
		psp.OnLinePower()
	}
}

func (psp *powerSavePlan) syncBrightnessData(value string) {
	psp.multiBrightnessWithPsm.init()
	err := psp.multiBrightnessWithPsm.toObject(value)
	if err != nil {
		logger.Warning(err)
	}
	psp.brightnessSave.Set(value)
}

func (psp *powerSavePlan) Start() error {
	psp.Reset()
	psp.initSettingsChangedHandler()

	gs := gio.NewSettings(gsSchemaPower)
	psp.brightnessSave.Bind(gs, settingKeySaveBrightnessWhilePsm)
	psp.multiBrightnessWithPsm = newMultiBrightnessWithPsm()

	helper := psp.manager.helper
	power := helper.Power
	display := helper.Display
	screenSaver := helper.ScreenSaver

	data, _ := power.PowerSavingModeBrightnessData().Get(0)
	if data == "" {
		// 有system级缓存数据时不需要从本地读，后面会同步。
		psp.initMultiBrightnessWithPsm()
	}

	// OnBattery changed will effect current PowerSavePlan
	err := power.OnBattery().ConnectChanged(func(hasValue bool, value bool) {
		psp.Reset()
	})
	if err != nil {
		logger.Warning("failed to connectChanged OnBattery:", err)
	}
	err = power.PowerSavingModeEnabled().ConnectChanged(psp.handlePowerSavingModeChanged)
	if err != nil {
		logger.Warning("failed to connectChanged PowerSavingModeEnabled:", err)
	}
	err = power.PowerSavingModeBrightnessDropPercent().ConnectChanged(psp.handlePowerSavingModeBrightnessDropPercentChanged) // 监听自动降低亮度的属性的改变
	if err != nil {
		logger.Warning("failed to connectChanged PowerSavingModeBrightnessDropPercent:", err)
	}
	err = power.PowerSavingModeBrightnessData().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		if psp.manager.isSessionActive() {
			return
		}
		// 非激活用户通过此属性同步激活用户配置的数据
		psp.syncBrightnessData(value)
	})
	if err != nil {
		logger.Warning("failed to connectChanged PowerSavingModeBrightnessData:", err)
	}
	sessionType := os.Getenv("XDG_SESSION_TYPE")
	if strings.Contains(sessionType, "wayland") {
		err = psp.ConnectIdle()
		if err != nil {
			logger.Warning("failed to ConnectIdleOn:", err)
		}
	} else {
		_, err = screenSaver.ConnectIdleOn(psp.HandleIdleOn)
		if err != nil {
			logger.Warning("failed to ConnectIdleOn:", err)
		}
		_, err = screenSaver.ConnectIdleOff(psp.HandleIdleOff)
		if err != nil {
			logger.Warning("failed to ConnectIdleOff:", err)
		}
	}
	err = display.Brightness().ConnectChanged(psp.handleBrightnessPropertyChanged)
	if err != nil {
		logger.Warning("failed to connectChanged Brightness:", err)
	}

	if data != "" {
		psp.syncBrightnessData(data)
		state, _ := power.PowerSavingModeEnabled().Get(0)
		psp.manager.settings.SetBoolean(settingKeyPowerSavingEnabled, state)
	} else {
		psp.dealWithPowerSavingModeWhenSystemBoot()
	}
	return nil
}

// 处理开关机前后，节能模式状态不一致的情况
func (psp *powerSavePlan) dealWithPowerSavingModeWhenSystemBoot() {
	power := psp.manager.helper.Power
	newPowerSaveState, _ := power.PowerSavingModeEnabled().Get(0)
	if newPowerSaveState != psp.manager.settings.GetBoolean(settingKeyPowerSavingEnabled) {
		psp.handlePowerSavingModeChanged(true, newPowerSaveState)
	}
}

// 分级以前逻辑有问题，目前先按无分级处理，保留该判断方式后续观望
func (psp *powerSavePlan) powerSavingModeIsMultiLevelAdjustment(maxBacklightBrightness float64) bool {
	const (
		multiLevelAdjustmentThreshold = 100 // 分级调节判断阈值，最大亮度值小于该值且不为0时，调节方式为分级调节
	)
	// 判断亮度调节方式是分级调节还是百分比滑动：最大亮度小于100且最大亮度不为0时，为分级调节
	return maxBacklightBrightness < multiLevelAdjustmentThreshold && maxBacklightBrightness != 0
}

// 节能模式亮度统一策略处理：根据当前非节能模式亮度和自动降低亮度比例配置，计算出节能模式需要降低的亮度
func (psp *powerSavePlan) powerSavingModeBrightnessDrop(brightness, scale float64) float64 {
	newBrightness := math.Round(brightness * 100 * (1 - scale/100))
	newBrightness = newBrightness / 100
	if newBrightness > 1.0 {
		newBrightness = 1.0
	} else if newBrightness < 0.1 {
		newBrightness = 0.1
	}
	return newBrightness
}

// 节能模式亮度统一策略处理：根据当前节能模式下的亮度和自动降低亮度比例配置，计算出恢复到非节能模式后的亮度
func (psp *powerSavePlan) powerSavingModeBrightnessRestored(brightness, scale float64) float64 {
	newBrightness := math.Round(brightness * 100 / (1 - scale/100))
	newBrightness = newBrightness / 100
	if newBrightness > 1.0 {
		newBrightness = 1.0
	} else if newBrightness < 0.1 {
		newBrightness = 0.1
	}
	return newBrightness
}

// 节能模式降低亮度的比例,并降低亮度
func (psp *powerSavePlan) handlePowerSavingModeBrightnessDropPercentChanged(hasValue bool, lowerValue uint32) {
	if !hasValue {
		return
	}
	logger.Debug("power saving mode lower brightness changed to", lowerValue)
	psp.manager.PropsMu.RLock()
	hasLightSensor := psp.manager.HasAmbientLightSensor
	psp.manager.PropsMu.RUnlock()
	if hasLightSensor && psp.manager.AmbientLightAdjustBrightness.Get() {
		return
	}
	if !psp.manager.isSessionActive() { // 系统级的调节保证只有激活用户才能做逻辑
		return
	}

	brightnessTable, err := psp.manager.helper.Display.GetBrightness(0)
	if err != nil {
		logger.Warning(err)
		return
	}
	oldLowerBrightnessScale := float64(psp.manager.savingModeBrightnessDropPercent.Get())
	newLowerBrightnessScale := float64(lowerValue)
	psp.manager.savingModeBrightnessDropPercent.Set(int32(lowerValue))
	savingModeEnable, err := psp.manager.helper.Power.PowerSavingModeEnabled().Get(0)
	if err != nil {
		logger.Error("get current power savingMode state error : ", err)
	}

	if savingModeEnable {
		for key, value := range brightnessTable {
			value = psp.powerSavingModeBrightnessRestored(value, oldLowerBrightnessScale)
			value = psp.powerSavingModeBrightnessDrop(value, newLowerBrightnessScale)
			brightnessTable[key] = value
			psp.psmPercentChangedTime = time.Now()
		}
	} else {
		// else中(非节能状态下的调节)不需要做响应,需要降低亮度的预设值在之前已经保存了
		return
	}
	psp.manager.setAndSaveDisplayBrightness(brightnessTable)
}

// 节能模式变化后的亮度修改
func (psp *powerSavePlan) handlePowerSavingModeChanged(hasValue bool, enabled bool) {
	if !hasValue {
		return
	}
	logger.Debug("power saving mode enabled changed to", enabled)

	psp.manager.settings.SetBoolean(settingKeyPowerSavingEnabled, enabled)

	if !psp.manager.isSessionActive() { // 系统级的调节保证只有激活用户才能做逻辑
		return
	}

	bInBootTime, err := psp.manager.helper.Power.IsInBootTime().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	if bInBootTime {
		logger.Debug("handlePowerSavingModeChanged InBootTime, Can't change brightness.")
		return
	}

	psp.manager.PropsMu.RLock()
	hasLightSensor := psp.manager.HasAmbientLightSensor
	psp.manager.PropsMu.RUnlock()

	if hasLightSensor && psp.manager.AmbientLightAdjustBrightness.Get() {
		return
	}

	brightnessTable, err := psp.manager.helper.Display.GetBrightness(0)
	if err != nil {
		logger.Warning(err)
		return
	}
	brightnessScale := float64(psp.manager.savingModeBrightnessDropPercent.Get())
	for key, value := range brightnessTable {
		if enabled {
			value = psp.powerSavingModeBrightnessDrop(value, brightnessScale)
		} else {
			value = psp.powerSavingModeBrightnessRestored(value, brightnessScale)
		}
		brightnessTable[key] = value
	}

	if enabled {
		psp.multiBrightnessWithPsm.init()
		psp.setBrightnessFromDisplay()
		psp.multiBrightnessWithPsm.mapToObject()
		psp.setToBrightnessSave()
		psp.psmEnabledTime = time.Now()
	} else {
		psp.brightnessSave.Set("")
	}
	psp.manager.setAndSaveDisplayBrightness(brightnessTable)
}

// 取消之前的任务
func (psp *powerSavePlan) interruptTasks() {
	psp.tasks.CancelAll()
	psp.tasks.Wait(10*time.Millisecond, 200)
	psp.tasks = nil
}

func (psp *powerSavePlan) Destroy() {
	psp.interruptTasks()
}

func (psp *powerSavePlan) addTaskNoLock(t *delayedTask) {
	psp.tasks = append(psp.tasks, t)
}

func (psp *powerSavePlan) addTask(t *delayedTask) {
	psp.mu.Lock()
	psp.addTaskNoLock(t)
	psp.mu.Unlock()
}

type metaTask struct {
	delay     int32
	realDelay time.Duration
	name      string
	fn        func()
}

type metaTasks []metaTask

func (mts metaTasks) min() int32 {
	if len(mts) == 0 {
		return 0
	}

	min := mts[0].delay
	for _, t := range mts[1:] {
		if t.delay < min {
			min = t.delay
		}
	}
	return min
}

func (mts metaTasks) setRealDelay(min int32) {
	for idx := range mts {
		t := &mts[idx]
		nSecs := t.delay - min
		if nSecs == 0 {
			t.realDelay = 1 * time.Millisecond
		} else {
			t.realDelay = time.Second * time.Duration(nSecs)
		}
	}
}

func (psp *powerSavePlan) Update(screenSaverStartDelay, lockDelay,
	screenBlackDelay, sleepDelay int32) {
	psp.mu.Lock()
	defer psp.mu.Unlock()

	psp.interruptTasks()
	logger.Debugf("update(screenSaverStartDelay=%vs, lockDelay=%vs,"+
		" screenBlackDelay=%vs, sleepDelay=%vs)",
		screenSaverStartDelay, lockDelay, screenBlackDelay, sleepDelay)

	// 按照优先级 待机=屏保>关闭显示器=自动锁屏
	tasks := make(metaTasks, 0, 5)

	if sleepDelay > 0 && canAddToTasks("sleep", sleepDelay, tasks) {
		tasks = append(tasks, metaTask{
			name:  "sleep",
			delay: sleepDelay,
			fn:    psp.makeSystemSleep,
		})
	}
	if screenSaverStartDelay > 0 && canAddToTasks("screenSaverStart", screenSaverStartDelay, tasks) {
		tasks = append(tasks, metaTask{
			name:  "screenSaverStart",
			delay: screenSaverStartDelay,
			fn:    psp.startScreensaver,
		})
	}

	if lockDelay > 0 {
		tasks = append(tasks, metaTask{
			name:  "lock",
			delay: lockDelay,
			fn:    psp.lock,
		})
	}

	if screenBlackDelay > 0 {
		tasks = append(tasks, metaTask{
			name:  "screenBlack",
			delay: screenBlackDelay,
			fn:    psp.screenBlack,
		})
	}

	min := tasks.min()
	tasks.setRealDelay(min)
	err := psp.setScreenSaverTimeout(min)
	if err != nil {
		logger.Warning("failed to set screen saver timeout:", err)
	}

	psp.metaTasks = tasks
}

func (psp *powerSavePlan) setScreenSaverTimeout(seconds int32) error {
	psp.screenSaverTimeout = seconds
	logger.Debugf("set ScreenSaver timeout to %d", seconds)
	err := psp.manager.helper.ScreenSaver.SetTimeout(0, uint32(seconds), 0, false)
	if err != nil {
		logger.Warningf("failed to set ScreenSaver timeout %d: %v", seconds, err)
	}
	return err
}

func (psp *powerSavePlan) saveCurrentBrightness() error {
	if psp.oldBrightnessTable == nil {
		var err error
		psp.oldBrightnessTable, err = psp.manager.helper.Display.Brightness().Get(0)
		if err != nil {
			return err
		}
		logger.Info("saveCurrentBrightness", psp.oldBrightnessTable)
		return nil
	}

	return errors.New("oldBrightnessTable is not nil")
}

func (psp *powerSavePlan) resetBrightness() {
	if psp.oldBrightnessTable != nil {
		logger.Debug("Reset all outputs brightness")
		psp.manager.setDisplayBrightness(psp.oldBrightnessTable)
		psp.oldBrightnessTable = nil
	}
}

func (psp *powerSavePlan) startScreensaver() {
	if os.Getenv("DESKTOP_CAN_SCREENSAVER") == "N" {
		logger.Info("do not start screensaver, env DESKTOP_CAN_SCREENSAVER == N")
		return
	}

	if !psp.allowScreenSaver {
		logger.Info("do not start screensaver, allowScreenSaver is false")
		return
	}

	startScreensaver()
	psp.screensaverRunning = true
}

func (psp *powerSavePlan) stopScreensaver() {
	if !psp.screensaverRunning {
		return
	}
	stopScreensaver()
	psp.screensaverRunning = false
}

func (psp *powerSavePlan) makeSystemSleep() {
	logger.Info("sleep")
	psp.stopScreensaver()
	// psp.manager.setDPMSModeOn()
	// psp.resetBrightness()
	psp.manager.doSuspendByFront()
}

func (psp *powerSavePlan) lock() {
	psp.manager.doLock(true)
}

// 降低显示器亮度，最终关闭显示器
func (psp *powerSavePlan) screenBlack() {
	manager := psp.manager
	logger.Info("Start screen black")

	adjustBrightnessEnabled := manager.settings.GetBoolean(settingKeyAdjustBrightnessEnabled)

	if adjustBrightnessEnabled {
		err := psp.saveCurrentBrightness()
		if err != nil {
			adjustBrightnessEnabled = false
			logger.Warning(err)
		} else {
			// half black
			brightnessTable := make(map[string]float64)
			brightnessRatio := 0.5
			logger.Debug("brightnessRatio:", brightnessRatio)
			for output, oldBrightness := range psp.oldBrightnessTable {
				brightnessTable[output] = oldBrightness * brightnessRatio
			}
			manager.setDisplayBrightness(brightnessTable)
		}
	} else {
		logger.Debug("adjust brightness disabled")
	}

	// full black
	const fullBlackTime = 5000 * time.Millisecond
	taskF := newDelayedTask("screenFullBlack", fullBlackTime, func() {
		psp.stopScreensaver()
		logger.Info("Screen full black")
		if manager.ScreenBlackLock.Get() {
			manager.lockWaitShow(5*time.Second, true)
		}

		if adjustBrightnessEnabled {
			// set min brightness for all outputs
			brightnessTable := make(map[string]float64)
			for output := range psp.oldBrightnessTable {
				brightnessTable[output] = 0.02
			}
			manager.setDisplayBrightness(brightnessTable)
		}
		manager.setDPMSModeOff()

	})
	psp.addTask(taskF)
}

func (psp *powerSavePlan) shouldPreventIdle() (bool, error) {
	conn := psp.manager.helper.xConn
	activeWin, err := ewmh.GetActiveWindow(conn).Reply(conn)
	if err != nil {
		return false, err
	}

	isFullscreenAndFocused, err := psp.isWindowFullScreenAndFocused(activeWin)
	if err != nil {
		return false, err
	}

	if !isFullscreenAndFocused {
		return false, nil
	}

	pid, err := ewmh.GetWMPid(conn, activeWin).Reply(conn)
	if err != nil {
		return false, err
	}

	p := procfs.Process(pid)
	cmdline, err := p.Cmdline()
	if err != nil {
		return false, err
	}

	for _, arg := range cmdline {
		for _, app := range psp.fullscreenWorkaroundAppList {
			if strings.Contains(arg, app) {
				logger.Debugf("match %q", app)
				return true, nil
			}
		}
	}
	return false, nil
}

// 开始 Idle
func (psp *powerSavePlan) HandleIdleOn() {
	psp.mu.Lock()
	defer psp.mu.Unlock()

	if psp.manager.shouldIgnoreIdleOn() {
		logger.Info("HandleIdleOn : IGNORE =========")
		return
	}

	if !psp.manager.isSessionActive() {
		logger.Info("X11 session is inactive, don't HandleIdleOn")
		return
	}

	logger.Info("HandleIdleOn")

	// check window, only x11 is supported, not apply to wayland
	if !psp.manager.UseWayland {
		preventIdle, err := psp.shouldPreventIdle()
		if err != nil {
			logger.Warning(err)
		}
		if preventIdle {
			logger.Debug("prevent idle")
			err := psp.manager.helper.ScreenSaver.SimulateUserActivity(0)
			if err != nil {
				logger.Warning(err)
			}
			return
		}

		idleTime := psp.metaTasks.min()
		xConn := psp.manager.helper.xConn
		xDefaultScreen := xConn.GetDefaultScreen()
		if xDefaultScreen != nil {
			xInfo, err := xscreensaver.QueryInfo(xConn, x.Drawable(xDefaultScreen.Root)).Reply(xConn)
			if err == nil {
				idleTime = int32(xInfo.MsSinceUserInput / 1000)
			} else {
				logger.Warning(err)
			}
		} else {
			logger.Warning("cannot get X11 default screen")
		}

		logger.Debugf("idle time: %d ms", idleTime)
		psp.metaTasks.setRealDelay(idleTime)
	}
	psp.modeBeforeIdle, _ = psp.manager.helper.Power.Mode().Get(0)

	for _, t := range psp.metaTasks {
		logger.Debugf("do %s after %v", t.name, t.realDelay)
		task := newDelayedTask(t.name, t.realDelay, t.fn)
		psp.addTaskNoLock(task)
	}

	_, err := os.Stat("/etc/deepin/no_suspend")
	if err == nil {
		if psp.manager.ScreenBlackLock.Get() {
			// m.setDPMSModeOn()
			// m.lockWaitShow(4 * time.Second)
			psp.manager.doLock(true)
			time.Sleep(time.Millisecond * 500)
		}
	}
}

func (ps *powerSavePlan) restoreDpmsStateFile() {
	v, err := ioutil.ReadFile("/tmp/dpms-state")
	if err != nil {
		return
	}

	if string(v) == "1" {
		err = ioutil.WriteFile("/tmp/dpms-state", []byte("0"), 0644)
		if err != nil {
			logger.Warning("WriteFile /tmp/dpms-state:", err)
		}
	}
}

func (psp *powerSavePlan) handleIdleOff() {
	psp.mu.Lock()
	defer psp.mu.Unlock()

	if psp.manager.shouldIgnoreIdleOff() {
		psp.manager.setPrepareSuspend(suspendStateFinish)
		logger.Info("HandleIdleOff : IGNORE =========")
		return
	}

	if psp.modeBeforeIdle != "" && psp.modeBeforeIdle != "performance" {
		err := psp.manager.helper.Power.SetMode(0, psp.modeBeforeIdle)
		if err != nil {
			logger.Warning(err)
		}
		psp.modeBeforeIdle = ""
	}

	psp.manager.setPrepareSuspend(suspendStateFinish)
	logger.Info("HandleIdleOff")
	psp.interruptTasks()
	psp.manager.setDPMSModeOn()
	psp.manager.setDDEBlackScreenActive(false)
	psp.resetBrightness()
	psp.restoreDpmsStateFile()
}

// 结束 Idle
func (psp *powerSavePlan) HandleIdleOff() {
	var powerPressAction int32
	if psp.manager.OnBattery {
		powerPressAction = psp.manager.BatteryPressPowerBtnAction.Get()
	} else {
		powerPressAction = psp.manager.LinePowerPressPowerBtnAction.Get()
	}

	if powerPressAction == powerActionTurnOffScreen {
		var delayHandleIdleOffInterval uint32
		delayHandleIdleOffInterval = psp.manager.delayHandleIdleOffIntervalWhenScreenBlack
		if delayHandleIdleOffInterval > 2500 {
			// 当“按电源按钮时-关闭显示器”时，最长可增加2.5S延时
			delayHandleIdleOffInterval = 2500
		}
		time.AfterFunc(time.Duration(delayHandleIdleOffInterval)*time.Millisecond, func() {
			psp.handleIdleOff()
		})
		return
	}

	psp.handleIdleOff()
}

func (psp *powerSavePlan) isWindowFullScreenAndFocused(xid x.Window) (bool, error) {
	conn := psp.manager.helper.xConn
	states, err := ewmh.GetWMState(conn, xid).Reply(conn)
	if err != nil {
		return false, err
	}
	found := 0
	for _, s := range states {
		if s == psp.atomNetWMStateFullscreen {
			found++
		}
		// 后端的代码之前是基于deepin-wm这个窗口适配，现在换成了kwin,state中没有 focus 这个属性了
		// else if s == psp.atomNetWMStateFocused {
		//	found++
		// }
		if found == 1 {
			return true, nil
		}
	}
	return false, nil
}

func (psp *powerSavePlan) setBrightnessFromDisplay() {
	d := psp.manager.helper.Display
	var err error
	psp.multiBrightnessWithPsm.valueTmp, err = d.Brightness().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}
}

func (psp *powerSavePlan) initMultiBrightnessWithPsm() {
	b := psp.brightnessSave.Get()

	err := psp.multiBrightnessWithPsm.toObject(b)
	if err != nil {
		logger.Warning(err)
	}

	psp.setBrightnessFromDisplay()
	// 开机重启注销等操作后，去掉手动调整亮度的限制条件，关闭节能模式亮度提升
	brightnessDropPercent, err := psp.manager.helper.Power.PowerSavingModeBrightnessDropPercent().Get(0)
	if err != nil {
		logger.Warning("failed to get PowerSavingModeBrightnessDropPercent:", err)
		return
	}
	for _, val := range psp.multiBrightnessWithPsm.MultiBrightness {
		if val.ManuallyModified {
			val.ManuallyModified = false
			val.BrightnessSaved = val.BrightnessLatest / (1 - float64(brightnessDropPercent)/100)
			if val.BrightnessSaved > 1 {
				val.BrightnessSaved = 1
			}
		}
	}
	psp.setToBrightnessSave()
}

func (psp *powerSavePlan) setToBrightnessSave() {
	data, err := psp.multiBrightnessWithPsm.toString()
	if err != nil {
		logger.Warning(err)
	}
	psp.brightnessSave.Set(data)
	psp.manager.helper.Power.PowerSavingModeBrightnessData().Set(0, data)
}

func (psp *powerSavePlan) handleBrightnessPropertyChanged(value bool, value2 map[string]float64) {
	if !value {
		return
	}

	p := psp.manager.helper.Power
	psmEnabled, err := p.PowerSavingModeEnabled().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}

	if psmEnabled {
		defer func() {
			psp.multiBrightnessWithPsm.valueTmp = value2
		}()

		now := time.Now()
		if now.Sub(psp.psmEnabledTime) < time.Second*2 {
			return
		}
		if now.Sub(psp.psmPercentChangedTime) < time.Second*2 {
			return
		}

		// 切换用户会配置亮度，这里加上会话激活的时间做为是否手动调节亮度的判断
		var t time.Time
		psp.manager.PropsMu.Lock()
		t = psp.manager.sessionActiveTime
		psp.manager.PropsMu.Unlock()

		if now.Sub(t) < time.Second*3 {
			return
		}

		changed := psp.multiBrightnessWithPsm.checkBrightnessChanged(value2)
		if changed {
			psp.setToBrightnessSave()
		}

	}
}

type brightnessWithPsp struct {
	MonitorName      string
	BrightnessSaved  float64 // 开启节能模式之前的亮度值
	BrightnessLatest float64 // 开启节能模式之后每次手动设置的亮度
	ManuallyModified bool
}

type multiBrightnessWithPsm struct {
	MultiBrightness []*brightnessWithPsp
	valueTmp        map[string]float64
}

func newMultiBrightnessWithPsm() *multiBrightnessWithPsm {
	return &multiBrightnessWithPsm{valueTmp: make(map[string]float64)}
}

func (mb *multiBrightnessWithPsm) init() {
	mb.MultiBrightness = mb.MultiBrightness[:0]
}

func (mb *multiBrightnessWithPsm) setBrightnessManuallyModified(name string, m bool, val float64) {
	for i, b := range mb.MultiBrightness {
		if b.MonitorName == name {
			mb.MultiBrightness[i].ManuallyModified = m
			mb.MultiBrightness[i].BrightnessLatest = val
			break
		}
	}
}

// 检查哪一个屏幕的亮度改变了
func (mb *multiBrightnessWithPsm) checkBrightnessChanged(data map[string]float64) bool {
	var changed bool
	for k, v := range data {
		if vTmp, ok := mb.valueTmp[k]; ok {
			if !isFloatEqual(v, vTmp) {
				changed = true
				mb.setBrightnessManuallyModified(k, true, v)
			}
		}
	}

	return changed
}

func (mb *multiBrightnessWithPsm) toString() (string, error) {
	bytes, err := json.Marshal(mb.MultiBrightness)
	if err != nil {
		return "", err
	}
	return string(bytes), nil
}

func (mb *multiBrightnessWithPsm) toObject(b string) error {
	if b == "" {
		return nil
	}
	err := json.Unmarshal([]byte(b), &mb.MultiBrightness)
	if err != nil {
		return err
	}

	return nil
}

func (mb *multiBrightnessWithPsm) mapToObject() {
	for k, v := range mb.valueTmp {
		mb.MultiBrightness = append(mb.MultiBrightness, &brightnessWithPsp{MonitorName: k, BrightnessSaved: v})
	}
}

// 判断休眠、待机、屏保、锁屏、关闭显示器等任务能否加入任务队列
// 优先级为：休眠 > 待机=屏保 > 锁屏=关闭显示器
func canAddToTasks(sType string, delay int32, tasks metaTasks) bool {
	if len(tasks) == 0 {
		return true
	}

	switch sType {
	case "hibernate":
		return true
	case "sleep":
		if delay < tasks.min() {
			return true
		} else {
			return false
		}
	case "screenSaverStart":
		if delay < tasks.min() {
			return true
		} else if delay == tasks.min() && tasks[len(tasks)-1].name == "sleep" {
			return true
		} else {
			return false
		}

	case "lock":
		if delay < tasks.min() {
			return true
		} else {
			return false
		}
	case "screenBlack":
		if delay < tasks.min() {
			return true
		} else if delay == tasks.min() && tasks[len(tasks)-1].name == "lock" {
			return true
		} else {
			return false
		}
	default:
		return false
	}
}

func (psp *powerSavePlan) ConnectIdle() error {
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return err
	}
	err = sessionBus.Object("org.deepin.dde.KWayland1",
		"/org/deepin/dde/KWayland1/Output").AddMatchSignal("org.deepin.dde.KWayland1.Idle", "IdleTimeout").Err
	if err != nil {
		logger.Warning(err)
		return err
	}
	sessionSigLoop := dbusutil.NewSignalLoop(sessionBus, 10)
	sessionSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.deepin.dde.KWayland1.Idle.IdleTimeout",
	}, func(sig *dbus.Signal) {
		if strings.HasPrefix(string(sig.Path),
			"/org/deepin/dde/KWayland1/") &&
			len(sig.Body) == 1 {
			bIdle, ok := sig.Body[0].(bool)
			if !ok {
				return
			}
			if bIdle {
				psp.HandleIdleOn()
			} else {
				psp.HandleIdleOff()
			}
		}
	})
	sessionSigLoop.Start()

	return nil
}

func (psp *powerSavePlan) initDsgConfig() error {
	logger.Info("initDsgConfig.")
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	psp.systemSigLoop = dbusutil.NewSignalLoop(systemBus, 10)
	// dsg 配置
	ds := ConfigManager.NewConfigManager(psp.systemSigLoop.Conn())
	dsPowerPath, err := ds.AcquireManager(0, dsettingsAppID, dsettingsPowerName, "")
	if err != nil {
		return err
	}
	dsPower, err := ConfigManager.NewManager(psp.systemSigLoop.Conn(), dsPowerPath)
	if err != nil {
		return err
	}
	psp.dsgPower = dsPower

	getAllowScreenSaver := func() {
		data, err := dsPower.Value(0, dsettingsAllowScreenSaver)
		if err != nil {
			logger.Warning(err)
			return
		}
		psp.allowScreenSaver = data.Value().(bool)
		logger.Info("allow screen saver enabled : ", psp.allowScreenSaver)
	}

	getAllowScreenSaver()

	getDelayWakeupInterval := func() {
		v, err := dsPower.Value(0, dsettingsDelayWakeupInterval)
		if err != nil {
			logger.Warning(err)
			return
		}
		switch vv := v.Value().(type) {
		case float64:
			psp.manager.delayWakeupInterval = uint32(vv)
		case int64:
			psp.manager.delayWakeupInterval = uint32(vv)
		default:
			logger.Warning("type is wrong!")
		}
		logger.Info("delay wake up interval : ", psp.manager.delayWakeupInterval)
	}

	getDelayWakeupInterval()

	getDelayHandleIdleOffIntervalWhenScreenBlack := func() {
		v, err := dsPower.Value(0, dsettingsDelayHandleIdleOffIntervalWhenScreenBlack)
		if err != nil {
			logger.Warning(err)
			return
		}
		switch vv := v.Value().(type) {
		case float64:
			psp.manager.delayHandleIdleOffIntervalWhenScreenBlack = uint32(vv)
		case int64:
			psp.manager.delayHandleIdleOffIntervalWhenScreenBlack = uint32(vv)
		default:
			logger.Warning("type is wrong!")
		}
		logger.Info("delay wake up interval : ", psp.manager.delayHandleIdleOffIntervalWhenScreenBlack)
	}
	getDelayHandleIdleOffIntervalWhenScreenBlack()

	dsPower.InitSignalExt(psp.systemSigLoop, true)
	dsPower.ConnectValueChanged(func(key string) {
		logger.Info("DSG org.deepin.dde.daemon.power valueChanged, key : ", key)
		switch key {
		case dsettingsAllowScreenSaver:
			getAllowScreenSaver()
		case dsettingsDelayWakeupInterval:
			getDelayWakeupInterval()
		case dsettingsDelayHandleIdleOffIntervalWhenScreenBlack:
			getDelayHandleIdleOffIntervalWhenScreenBlack()
		default:
		}
	})

	return nil
}
