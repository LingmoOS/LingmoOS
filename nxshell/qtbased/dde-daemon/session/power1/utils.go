// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"io/ioutil"
	"math"
	"os/exec"
	"strings"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-api/soundutils"
	. "github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/gsettings"
	"github.com/linuxdeepin/go-lib/pulse"
	"github.com/linuxdeepin/go-x11-client/ext/dpms"
)

const (
	dpmsStateOn int32 = iota
	dpmsStateStandBy
	dpmsStateSuspend
	dpmsStateOff
)

func (m *Manager) waitLockShowing(timeout time.Duration) {
	ticker := time.NewTicker(time.Millisecond * 300)
	timer := time.NewTimer(timeout)
	for {
		select {
		case _, ok := <-ticker.C:
			if !ok {
				logger.Error("Invalid ticker event")
				return
			}

			logger.Debug("waitLockShowing tick")
			locked, err := m.helper.SessionManager.Locked().Get(0)
			if err != nil {
				logger.Warning(err)
				continue
			}
			if locked {
				logger.Debug("waitLockShowing found")
				ticker.Stop()
				return
			}

		case <-timer.C:
			logger.Debug("waitLockShowing failed timeout!")
			ticker.Stop()
			return
		}
	}
}

func (m *Manager) lockWaitShow(timeout time.Duration, autoStartAuth bool) {
	m.doLock(autoStartAuth)
	if m.UseWayland {
		logger.Debug("In wayland environment, unsupported check lock whether showin")
		return
	}
	m.waitLockShowing(timeout)
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
	if m.delayInActive {
		return
	}

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

func (m *Manager) setDDEBlackScreenActive(active bool) {
	if m.delayInActive {
		return
	}

	if !m.SleepLock.Get() {
		logger.Info("setDDEBlackScreenActive GSettings of sleep-lock is false, Not use dde black widget")
		return
	}

	logger.Info("set blackScreen effect active: ", active)
	bus, err := dbus.SessionBus()
	if err == nil {
		dbusObject := bus.Object("com.deepin.dde.BlackScreen", "/com/deepin/dde/BlackScreen")
		err = dbusObject.Call("com.deepin.dde.BlackScreen.setActive", 0, active).Err
		if err != nil {
			logger.Warning("set blackScreen active failed:", err)
		}
	}

	blackscreenCmd := "dbus-send --print-reply --dest=org.kde.KWin /BlackScreen org.kde.kwin.BlackScreen.setActive boolean:"
	if active {
		blackscreenCmd = blackscreenCmd + "true"
	} else {
		blackscreenCmd = blackscreenCmd + "false"
	}
	cmd := exec.Command("/bin/bash", "-c", blackscreenCmd)
	err = cmd.Run()
	if err != nil {
		logger.Warning("Set wm blackscreen failed", err)
	}
}

func (m *Manager) getDPMSMode() int32 {
	logger.Debug("get DPMS Mode")

	var err error
	var mode int32

	if m.UseWayland {
		mode, err = m.getDpmsModeByKwin()
	} else {
		var dpmsInfo *dpms.InfoReply
		c := m.helper.xConn
		dpmsInfo, err = dpms.Info(c).Reply(c)
		if err != nil {
			mode = int32(dpmsInfo.PowerLevel)
		}
	}

	if err != nil {
		logger.Warning("get DPMS mode error:", err)
	}

	return mode
}

var prevTimestamp int64

func (m *Manager) setDPMSModeOn() {
	if m.delayHandleIdleOffIntervalWhenScreenBlack == 0 {
		timestamp := time.Now().UnixNano()
		tmp := timestamp - prevTimestamp
		logger.Debug("[setDPMSModeOn] timestamp:", prevTimestamp, timestamp, tmp)
		prevTimestamp = timestamp
		if tmp < 300000000 {
			logger.Debug("[setDPMSModeOn] div < 300ms ignored.")
			return
		}
	}

	logger.Info("DPMS On")

	autoWm := m.getAutoChangeDeepinWm()
	if autoWm {
		m.tryChangeDeepinWM()
	}

	var err error
	if m.UseWayland {
		err = m.setDpmsModeByKwin(dpmsStateOn)
	} else {
		c := m.helper.xConn
		err = dpms.ForceLevelChecked(c, dpms.DPMSModeOn).Check(c)
	}

	if err != nil {
		logger.Warning("set DPMS on error:", err)
	}

	if autoWm {
		m.setAutoChangeDeepinWm(false)
	}
}

func (m *Manager) setDPMSModeOff() {
	logger.Info("DPMS Off")

	var err error
	if m.UseWayland {
		err = m.setDpmsModeByKwin(dpmsStateOff)
	} else {
		c := m.helper.xConn
		err = dpms.ForceLevelChecked(c, dpms.DPMSModeOff).Check(c)
	}
	if err != nil {
		logger.Warning("set DPMS off error:", err)
	}
	ioutil.WriteFile("/tmp/dpms-state", []byte("1"), 0644)
}

const (
	lockFrontServiceName = "org.deepin.dde.LockFront1"
	lockFrontIfc         = lockFrontServiceName
	lockFrontObjPath     = "/org/deepin/dde/LockFront1"
)

func (m *Manager) tryChangeDeepinWM() bool {
	ret := false
	count := 0
	for {
		if count > 2 {
			break
		}
		count++

		enabled, err := m.wmDBus.CompositingEnabled().Get(0)
		if err != nil {
			logger.Warning("tryChangeDeepinWM get CompositingEnabled err : ", err)
			continue
		}
		if !enabled {
			err := m.wmDBus.CompositingEnabled().Set(0, true)
			if err != nil {
				logger.Warning("tryChangeDeepinWM set CompositingEnabled true, err :", err)
				continue
			}

			// dde-osd需要使用该dconfig值，true不弹该特效osd
			ret = true
			break
		}
	}

	logger.Info(" tryChangeDeepinWM ret : ", ret)
	return ret
}

func (m *Manager) doLock(autoStartAuth bool) {
	locked, err := m.sessionManager.Locked().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	if locked {
		logger.Info("Current is locked. ignore this doLock.")
		return
	}

	logger.Info("Lock Screen")
	bus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return
	}
	lockFrontObj := bus.Object(lockFrontServiceName, lockFrontObjPath)
	err = lockFrontObj.Call(lockFrontIfc+".ShowAuth", 0, autoStartAuth).Err
	if err != nil {
		logger.Warning("failed to call lockFront ShowAuth:", err)
	}
}

func (m *Manager) canSuspend() bool {
	can, err := m.helper.SessionManager.CanSuspend(0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return can
}

func (m *Manager) doSuspend() {
	if !m.canSuspend() {
		logger.Info("can not suspend")
		return
	}

	logger.Debug("suspend")
	err := m.helper.SessionManager.RequestSuspend(0)
	if err != nil {
		logger.Warning("failed to suspend:", err)
	}
}

// 为了处理待机闪屏的问题，通过前端进行待机，前端会在待机前显示一个纯黑的界面
func (m *Manager) doSuspendByFront() {
	if !m.canSuspend() {
		logger.Info("can not suspend")
		return
	}

	logger.Debug("suspend")
	err := m.helper.ShutdownFront.Suspend(0)
	if err != nil {
		logger.Warning("failed to suspend:", err)
	}
}

func (m *Manager) canShutdown() bool {
	can, err := m.helper.SessionManager.CanShutdown(0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return can
}

// 正常关机流程，存在 block or delay shutdown 或多用户时，显示shutdown界面，其他情况，直接关机
func (m *Manager) doShutdown() {
	if m.hasShutdownInhibit() || m.hasMultipleDisplaySession() {
		logger.Info("exist shutdown inhibit(delay or block) or multiple display session")
		err := m.ddeShutdown.Shutdown(0)
		if err != nil {
			logger.Warning(err)
		}
	} else {
		logger.Debug("Shutdown")
		err := m.helper.SessionManager.RequestShutdown(0)
		if err != nil {
			logger.Warning("failed to Shutdown:", err)
		}
	}
}

func (m *Manager) hasShutdownInhibit() bool {
	// 先检查是否有delay 或 block shutdown的inhibitor
	inhibitors, err := m.objLogin.ListInhibitors(0)
	if err != nil {
		logger.Warning("failed to call login ListInhibitors:", err)
	} else {
		for _, inhibit := range inhibitors {
			logger.Infof("inhibit is: %+v", inhibit)
			if strings.Contains(inhibit.What, "shutdown") {
				return true
			}
		}
	}
	return false
}

// 定时关机流程: 存在block关机项时，显示shutdown界面，无block时，直接关机，delay情况进行延迟关机
func (m *Manager) doAutoShutdown() {
	if m.hasShutdownBlock() {
		logger.Warning("Shutdown")
		err := m.ddeShutdown.Shutdown(0)
		if err != nil {
			logger.Warning(err)
		}
	} else {
		logger.Warning("Shutdown")
		err := m.helper.SessionManager.RequestShutdown(0)
		// m.lastShutdownTime = time.Now().Unix()
		// m.savePowerDsgConfig(dsettingLastShutdownTime)
		if err != nil {
			logger.Warning("failed to Shutdown:", err)
		}
	}
}

func (m *Manager) hasShutdownBlock() bool {
	// 检查是否有block shutdown的inhibitor
	inhibitors, err := m.objLogin.ListInhibitors(0)
	if err != nil {
		logger.Warning("failed to call login ListInhibitors:", err)
	} else {
		for _, inhibit := range inhibitors {
			logger.Infof("inhibit is: %+v", inhibit)
			if strings.Contains(inhibit.What, "shutdown") && inhibit.Mode == "block" {
				logger.Info("exist shutdown block")
				return true
			}
		}
	}
	return false
}

func (m *Manager) hasMultipleDisplaySession() bool {
	// 检查是否有多个图形session,有多个图形session就需要显示阻塞界面
	sessions, err := m.displayManager.Sessions().Get(0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return len(sessions) >= 2
}

func (m *Manager) canHibernate() bool {
	can, err := m.helper.SessionManager.CanHibernate(0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return can
}

func (m *Manager) doHibernate() {
	if !m.canHibernate() {
		logger.Info("can not hibernate")
		return
	}
	logger.Debug("hibernate")
	err := m.helper.SessionManager.RequestHibernate(0)
	if err != nil {
		logger.Warning("failed to hibernate:", err)
	}
}

func (m *Manager) doHibernateByFront() {
	if !m.canHibernate() {
		logger.Info("can not hibernate")
		return
	}

	logger.Debug("hibernate")
	err := m.helper.ShutdownFront.Hibernate(0)
	if err != nil {
		logger.Warning("failed to hibernate:", err)
	}
}

func (m *Manager) doTurnOffScreen() {
	if m.ScreenBlackLock.Get() {
		logger.Info("Show lock")
		m.doLock(true)
		time.Sleep(1 * time.Second)
	}

	logger.Info("Turn off screen")
	m.setDPMSModeOff()
}

func (m *Manager) setDisplayBrightness(brightnessTable map[string]float64) {
	display := m.helper.Display
	for output, brightness := range brightnessTable {
		logger.Infof("Change output %q brightness to %.2f", output, brightness)
		err := display.SetBrightness(0, output, brightness)
		if err != nil {
			logger.Warningf("Change output %q brightness to %.2f failed: %v", output, brightness, err)
		} else {
			logger.Infof("Change output %q brightness to %.2f done!", output, brightness)
		}
	}
}

func (m *Manager) setAndSaveDisplayBrightness(brightnessTable map[string]float64) {
	display := m.helper.Display
	for output, brightness := range brightnessTable {
		logger.Infof("Change output %q brightness to %.2f", output, brightness)
		err := display.SetAndSaveBrightness(0, output, brightness)
		if err != nil {
			logger.Warningf("Change output %q brightness to %.2f failed: %v", output, brightness, err)
		} else {
			logger.Infof("Change output %q brightness to %.2f done!", output, brightness)
		}
	}
}

func doShowDDELowPower() {
	logger.Info("Show dde low power")
	go func() {
		err := exec.Command(cmdDDELowPower, "--raise").Run()
		if err != nil {
			logger.Warning(err)
		}
	}()
}

func doCloseDDELowPower() {
	logger.Info("Close low power")
	go func() {
		err := exec.Command(cmdDDELowPower, "--quit").Run()
		if err != nil {
			logger.Warning(err)
		}
	}()
}

func (m *Manager) sendNotify(icon, summary, body string) {
	if !m.LowPowerNotifyEnable.Get() {
		logger.Info("notify switch is off ")
		return
	}
	n := m.helper.Notifications
	_, err := n.Notify(0, "dde-control-center", 0, icon, summary, body, nil, nil, -1)
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) sendChangeNotify(icon, summary, body string) {
	n := m.helper.Notifications
	_, err := n.Notify(0, "dde-control-center", 0, icon, summary, body, nil, nil, -1)
	if err != nil {
		logger.Warning(err)
	}
}

const iconBatteryLow = "notification-battery-low"

func playSound(name string) {
	logger.Debug("play system sound", name)
	go func() {
		err := soundutils.PlaySystemSound(name, "")
		if err != nil {
			logger.Warning(err)
		}
	}()
}

const (
	deepinScreensaverDBusServiceName = "com.deepin.ScreenSaver"
	deepinScreensaverDBusPath        = "/com/deepin/ScreenSaver"
	deepinScreensaverDBusInterface   = deepinScreensaverDBusServiceName
)

func startScreensaver() {
	logger.Info("start screensaver")
	bus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return
	}

	obj := bus.Object(deepinScreensaverDBusServiceName, deepinScreensaverDBusPath)
	err = obj.Call(deepinScreensaverDBusInterface+".Start", 0).Err
	if err != nil {
		logger.Warning(err)
	}
}

func stopScreensaver() {
	logger.Info("stop screensaver")
	bus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return
	}

	obj := bus.Object(deepinScreensaverDBusServiceName, deepinScreensaverDBusPath)
	err = obj.Call(deepinScreensaverDBusInterface+".Stop", 0).Err
	if err != nil {
		logger.Warning(err)
	}
}

// TODO(jouyouyun): move to common library
func suspendPulseSinks(suspend int) {
	var ctx = pulse.GetContext()
	if ctx == nil {
		logger.Error("Failed to connect pulseaudio server")
		return
	}
	for _, sink := range ctx.GetSinkList() {
		ctx.SuspendSinkById(sink.Index, suspend)
	}
}

// TODO(jouyouyun): move to common library
func suspendPulseSources(suspend int) {
	var ctx = pulse.GetContext()
	if ctx == nil {
		logger.Error("Failed to connect pulseaudio server")
		return
	}
	for _, source := range ctx.GetSourceList() {
		ctx.SuspendSourceById(source.Index, suspend)
	}
}

func (m *Manager) initGSettingsConnectChanged() {
	const powerSettingsIcon = "preferences-system"

	isIllegalAction := func(action int32) bool {
		return (action == powerActionHibernate && !m.canHibernate()) ||
			(action == powerActionSuspend && !m.canSuspend())
	}

	// 监听 session power 的属性的改变,并发送通知
	gsettings.ConnectChanged(gsSchemaPower, "*", func(key string) {
		logger.Debug("Power Settings Changed :", key)
		switch key {
		case settingKeyLinePowerLidClosedAction:
			value := m.LinePowerLidClosedAction.Get()
			if isIllegalAction(value) {
				break
			}
		case settingKeyLinePowerPressPowerBtnAction:
			value := m.LinePowerPressPowerBtnAction.Get()
			if isIllegalAction(value) {
				break
			}
		case settingKeyBatteryLidClosedAction:
			value := m.BatteryLidClosedAction.Get()
			if isIllegalAction(value) {
				break
			}
		case settingKeyBatteryPressPowerBtnAction:
			value := m.BatteryPressPowerBtnAction.Get()
			if isIllegalAction(value) {
				break
			}
		case settingKeyHighPerformanceEnabled:
			// 根据systemPower::IsHighPerformanceEnabled GSetting::settingKeyHighPerformanceEnabled
			bSettingKeyHighPerformanceEnabled := m.settings.GetBoolean(settingKeyHighPerformanceEnabled)
			if bSettingKeyHighPerformanceEnabled == m.gsHighPerformanceEnabled {
				return
			}
			m.gsHighPerformanceEnabled = bSettingKeyHighPerformanceEnabled
			isHighPerformanceSupported, _ := m.systemPower.IsHighPerformanceSupported().Get(0)
			m.setPropIsHighPerformanceSupported(isHighPerformanceSupported && bSettingKeyHighPerformanceEnabled)
		}
	})
}

func getPowerActionString(action int32) string {
	switch action {
	case powerActionShutdown:
		return Tr("your computer will shut down")
	case powerActionSuspend:
		return Tr("your computer will suspend")
	case powerActionHibernate:
		return Tr("your computer will hibernate")
	case powerActionTurnOffScreen:
		return Tr("your monitor will turn off")
	case powerActionShowShutdownInterface:
		return Tr("your monitor will show the shutdown interface")
	case powerActionDoNothing:
		return Tr("it will do nothing to your computer")
	}
	return ""
}

func isFloatEqual(f1, f2 float64) bool {
	return math.Abs(f1-f2) < 1e-6
}

func (m *Manager) getDpmsList() ([]dbus.Variant, error) {
	sessionBus := m.sessionSigLoop.Conn()
	sessionObj := sessionBus.Object("org.deepin.dde.KWayland1", "/org/deepin/dde/KWayland1/DpmsManager")
	var ret []dbus.Variant
	err := sessionObj.Call("org.deepin.dde.KWayland1.DpmsManager.dpmsList", 0).Store(&ret)
	if err != nil {
		logger.Warning(err)
		return nil, err
	}

	return ret, nil
}

func (m *Manager) getDpmsModeByKwin() (int32, error) {
	list, err := m.getDpmsList()
	if err != nil {
		logger.Warning(err)
		return dpmsStateOn, err
	}

	var dpmsMode int32
	for i := 0; i < len(list); i++ {
		v := list[i].Value().(string)
		sessionObj := m.sessionSigLoop.Conn().Object("org.deepin.dde.KWayland1", dbus.ObjectPath(v))
		err = sessionObj.Call("org.deepin.dde.KWayland1.Dpms.getDpmsMode", 0).Store(&dpmsMode)
		if err != nil {
			logger.Warning(err)
			return dpmsStateOn, err
		}
	}

	return dpmsMode, nil
}

func (m *Manager) setDpmsModeByKwin(mode int32) error {
	list, err := m.getDpmsList()
	if err != nil {
		logger.Warning(err)
		return err
	}

	for i := 0; i < len(list); i++ {
		v := list[i].Value().(string)
		sessionObj := m.sessionSigLoop.Conn().Object("org.deepin.dde.KWayland1", dbus.ObjectPath(v))
		err = sessionObj.Call("org.deepin.dde.KWayland1.Dpms.setDpmsMode", 0, int32(mode)).Err
		if err != nil {
			logger.Warning(err)
			return err
		}
	}

	return nil
}

func byteSliceEqual(v1, v2 []byte) bool {
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
