// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"io/ioutil"
	"os/exec"
	"strings"
	"time"

	"github.com/godbus/dbus/v5"
	power "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.power1"
)

// 按键码
// https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
// nolint
const (
	KEY_TOUCHPAD_TOGGLE = 0x212
	KEY_TOUCHPAD_ON     = 0x213
	KEY_TOUCHPAD_OFF    = 0x214
	KEY_POWER           = 116
	KEY_FN_ESC          = 0x1d1
	KEY_MICMUTE         = 248
	KEY_SETUP           = 141
	KEY_SCREENLOCK      = 152
	KEY_CYCLEWINDOWS    = 154
	KEY_MODE            = 0x175
	KEY_KBDILLUMTOGGLE  = 228
	KEY_RFKILL          = 247
	KEY_UNKNOWN         = 240
	KEY_CAMERA          = 212
)

type SpecialKeycodeMapKey struct {
	keycode      uint32 // 按键码
	pressed      bool   // true:按下事件，false松开事件
	ctrlPressed  bool   // ctrl 是否处于按下状态
	shiftPressed bool   // shift 是否处于按下状态
	altPressed   bool   // alt 是否处于按下状态
	superPressed bool   // super 是否处于按下状态
}

// 按键修饰
const (
	MODIFY_NONE uint32 = 0
	MODIFY_CTRL uint32 = 1 << iota
	MODIFY_SHIFT
	MODIFY_ALT
	MODIFY_SUPER
)

// 通过按键码和状态创建map的key
func createSpecialKeycodeIndex(keycode uint32, pressed bool, modify uint32) SpecialKeycodeMapKey {
	return SpecialKeycodeMapKey{
		keycode,
		pressed,
		(modify & MODIFY_CTRL) != 0,
		(modify & MODIFY_SHIFT) != 0,
		(modify & MODIFY_ALT) != 0,
		(modify & MODIFY_SUPER) != 0,
	}
}

// 初始化按键到处理函数的map
func (m *Manager) initSpecialKeycodeMap() {
	m.specialKeycodeBindingList = make(map[SpecialKeycodeMapKey]func())

	// 触摸板开关键
	key := createSpecialKeycodeIndex(KEY_TOUCHPAD_TOGGLE, true, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleTouchpadToggle

	// 触摸板开键
	key = createSpecialKeycodeIndex(KEY_TOUCHPAD_ON, true, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleTouchpadOn

	// 触摸板关键
	key = createSpecialKeycodeIndex(KEY_TOUCHPAD_OFF, true, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleTouchpadOff

	// 电源键，松开时触发
	key = createSpecialKeycodeIndex(KEY_POWER, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handlePower

	// FnLock
	key = createSpecialKeycodeIndex(KEY_FN_ESC, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleFnLock

	// 开关麦克风
	key = createSpecialKeycodeIndex(KEY_MICMUTE, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleMicMute

	// 打开控制中心
	key = createSpecialKeycodeIndex(KEY_SETUP, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleOpenControlCenter

	// 锁屏
	key = createSpecialKeycodeIndex(KEY_SCREENLOCK, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleScreenlock

	// 打开多任务视图
	key = createSpecialKeycodeIndex(KEY_CYCLEWINDOWS, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleWorkspace

	// 切换性能模式
	key = createSpecialKeycodeIndex(KEY_MODE, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleSwitchPowerMode

	// 切换键盘背光模式
	key = createSpecialKeycodeIndex(KEY_KBDILLUMTOGGLE, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleKbdLight

	// 切换飞行模式 Hard开/关状态
	key = createSpecialKeycodeIndex(KEY_RFKILL, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleRFKILL

	// 打开截屏
	key = createSpecialKeycodeIndex(294, false, MODIFY_NONE)
	m.specialKeycodeBindingList[key] = m.handleScreenshot

	// 打开设备管理器
	if m.deviceManagerControlEnable {
		key = createSpecialKeycodeIndex(KEY_UNKNOWN, false, MODIFY_NONE)
		m.specialKeycodeBindingList[key] = m.handleOpenDeviceManager
	}
}

// 处理函数的总入口
func (m *Manager) handleSpecialKeycode(keycode uint32,
	pressed bool,
	ctrlPressed bool,
	shiftPressed bool,
	altPressed bool,
	superPressed bool) {

	if keycode == KEY_CAMERA {
		if pressed {
			showOSD("CameraOn")
		} else {
			showOSD("CameraOff")
		}
		return
	}

	key := SpecialKeycodeMapKey{
		keycode,
		pressed,
		ctrlPressed,
		shiftPressed,
		altPressed,
		superPressed,
	}

	if keycode == KEY_UNKNOWN {
		if pressed {
			m.fnLockCount++
		} else {
			m.fnLockCount--
		}
	} else {
		m.fnLockCount = 0
	}

	handler, ok := m.specialKeycodeBindingList[key]
	if ok {
		handler()
	}
}

// 开关FnLock
func (m *Manager) handleFnLock() {
	showOSD("FnToggle")
}

// 开关麦克风
func (m *Manager) handleMicMute() {
	source, err := m.audioController.getDefaultSource()
	if err != nil {
		logger.Warning(err)
		return
	}

	mute, err := source.Mute().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}
	mute = !mute
	err = source.SetMute(0, mute)
	if err != nil {
		logger.Warning(err)
		return
	}

	var osd string
	if mute {
		osd = "AudioMicMuteOn"
	} else {
		osd = "AudioMicMuteOff"
	}
	showOSD(osd)
}

// 打开控制中心
func (m *Manager) handleOpenControlCenter() {
	cmd := "dbus-send --session --dest=com.deepin.dde.ControlCenter --print-reply /com/deepin/dde/ControlCenter com.deepin.dde.ControlCenter.Show"
	m.execCmd(cmd, false)
}

// 锁屏
func (m *Manager) handleScreenlock() {
	m.lockFront.Show(0)
}

// 打开任务视图
func (m *Manager) handleWorkspace() {
	logger.Info("handleWorkspace.")
	locked, err := m.sessionManager.Locked().Get(0)
	if err != nil {
		logger.Warning("sessionManager get locked error:", err)
	}
	if locked {
		logger.Info("Now LockScreen is locked.")
		return
	}
	m.wm.ShowWorkspace(0)
}

// 切换触摸板状态
func (m *Manager) handleTouchpadToggle() {
	showOSD("TouchpadToggle")
}

func (m *Manager) handleTouchpadOn() {
	showOSD("TouchpadOn")
}

func (m *Manager) handleTouchpadOff() {
	showOSD("TouchpadOff")
}

// 切换性能模式
func (m *Manager) handleSwitchPowerMode() {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning("connect to system bus failed:", err)
		return
	}

	pwr := power.NewPower(systemBus)
	mode, err := pwr.Mode().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}

	isHighPerformanceSupported, err := pwr.IsHighPerformanceSupported().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}

	bHighPerformanceEnabled := m.gsPower.GetBoolean("high-performance-enabled")
	logger.Info(" handleSwitchPowerMode, isHighPerformanceSupported : ", isHighPerformanceSupported)

	targetMode := ""
	//平衡 balance, 节能 powersave, 高性能 performance
	if isHighPerformanceSupported && bHighPerformanceEnabled {
		if mode == "balance" {
			targetMode = "powersave"
		} else if mode == "powersave" {
			targetMode = "performance"
		} else if mode == "performance" {
			targetMode = "balance"
		}
	} else {
		if mode == "balance" {
			targetMode = "powersave"
		} else if mode == "powersave" {
			targetMode = "balance"
		}
	}

	if targetMode == "" {
		return
	}
	err = pwr.SetMode(0, targetMode)

	logger.Infof("[handleSwitchPowerMode] from %s to %s", mode, targetMode)
	if err != nil {
		logger.Warning(err)
	} else {
		showOSD(targetMode)
	}
}

func (m *Manager) isWmBlackScreenActive() bool {
	bus, err := dbus.SessionBus()
	if err == nil {
		kwinInter := bus.Object("org.kde.KWin", "/BlackScreen")
		var active bool
		err = kwinInter.Call("org.kde.kwin.BlackScreen.getActive",
			dbus.FlagNoAutoStart).Store(&active)
		if err != nil {
			logger.Warning("failed to get kwin blackscreen effect active:", err)
			return false
		}
		return active
	} else {
		return false
	}
}

func (m *Manager) setWmBlackScreenActive(active bool) {
	logger.Info("set blackScreen effect active: ", active)
	bus, err := dbus.SessionBus()
	if err == nil {
		kwinInter := bus.Object("org.kde.KWin", "/BlackScreen")
		err = kwinInter.Call("org.kde.kwin.BlackScreen.setActive", 0, active).Err
		if err != nil {
			logger.Warning("set blackScreen active failed:", err)
		}
	}
}

// 电源键的处理
func (m *Manager) handlePower() {
	var powerPressAction int32

	systemBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning("connect to system bus failed:", err)
		return
	}
	systemPower := power.NewPower(systemBus)
	onBattery, err := systemPower.OnBattery().Get(0)
	if err != nil {
		logger.Warning(err)
	}

	screenBlackLock := m.gsPower.GetBoolean("screen-black-lock")

	if onBattery {
		powerPressAction = m.gsPower.GetEnum("battery-press-power-button")
	} else {
		powerPressAction = m.gsPower.GetEnum("line-power-press-power-button")
	}
	switch powerPressAction {
	case powerActionShutdown:
		m.systemShutdown()
	case powerActionSuspend:
		m.systemSuspendByFront()
	case powerActionHibernate:
		m.systemHibernateByFront()
	case powerActionTurnOffScreen:
		if screenBlackLock {
			systemLock()
		}
		m.systemTurnOffScreen()
	case powerActionShowUI:
		cmd := "/usr/lib/deepin-daemon/dde-shutdown.sh"
		go func() {
			locked, err := m.sessionManager.Locked().Get(0)
			if err != nil {
				logger.Warning("sessionManager get locked error:", err)
			}
			if !locked {
				err = m.execCmd(cmd, false)
				if err != nil {
					logger.Warning("execCmd error:", err)
				}
			}
		}()
	}
}

// 响应键盘背光模式切换
func (m *Manager) handleKbdLight() {
	data, err := ioutil.ReadFile("/sys/class/backlight/n70z/n70z_kbbacklight")
	if err != nil {
		logger.Warning(err)
		return
	}

	mode := strings.TrimSpace(string(data))
	switch mode {
	case "0":
		showOSD("KbLightClose")
	case "1":
		showOSD("KbLightAuto")
	case "2":
		showOSD("KbLightLow")
	case "3":
		showOSD("KbLightHigh")
	}
	logger.Debugf("switch kbd light mode to %v", mode)
}

const minCallRfkillInterval = 1500 * time.Millisecond

func runRfkillListAll() (bool, error) {
	cmd := exec.Command("rfkill", "list", "all")
	out, err := cmd.Output()
	return strings.Contains(string(out), "Hard blocked: yes"), err
}

// 监听 rfkill 开关状态
func (m *Manager) listenRfkill(callback func(status bool)) {
	for {
		// 执行 rfkill list 命令
		out, err := exec.Command("rfkill", "list").Output()
		if err != nil {
			logger.Warning(err)
		}

		// 解析 rfkill 状态
		status := strings.Contains(string(out), "Hard blocked: yes")
		if status != m.rfkillState {
			// 调用回调函数
			callback(status)
			m.rfkillState = status
		}
		m.repeatCount += 1
		if m.repeatCount > 5 {
			m.repeatCount = 0
			return
		}

		// 每秒检查一次 rfkill 状态
		time.Sleep(time.Second)
	}
}

func (m *Manager) handleRFKILL() {
	m.repeatCount = 0
	if m.delayUpdateRfTimer == nil {
		m.delayUpdateRfTimer = time.AfterFunc(minCallRfkillInterval, func() {
			m.listenRfkill(func(state bool) {
				m.airplane.EnableWifi(0, state)
				m.airplane.EnableBluetooth(0, state)
				if state {
					logger.Info("rfkill is on")
				} else {
					logger.Info("rfkill is off")
				}
			})
		})
	}
	m.emitSignalKeyEvent(true, "KEY_RFKILL")
	m.delayUpdateRfTimer.Stop()
	m.delayUpdateRfTimer.Reset(minCallRfkillInterval)
}

func (m *Manager) handleScreenshot() {
	m.execCmd("dbus-send --print-reply --dest=com.deepin.Screenshot /com/deepin/Screenshot com.deepin.Screenshot.StartScreenshot", false)
}

func (m *Manager) handleOpenDeviceManager() {
	if m.fnLockCount > 0 {
		// 连续 KEY_UNKNOWN 为切换 fn-lock 忽略掉
		m.fnLocking = true
		return
	}

	if m.fnLocking {
		m.fnLocking = false
		return
	}

	err := m.execCmd("deepin-devicemanager", true)
	if err != nil {
		logger.Warning(err)
	}
}
