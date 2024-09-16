// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"bytes"
	"errors"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/keybinding1/util"
	wm "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.wm"
	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/strv"
	"github.com/linuxdeepin/go-x11-client/ext/dpms"
)

// nolint
const (
	suspendStateUnknown = iota + 1
	suspendStateLidOpen
	suspendStateFinish
	suspendStateWakeup
	suspendStatePrepare
	suspendStateLidClose
	suspendStateButtonClick
)

type networkDevice struct {
	Udi                 string
	Path                dbus.ObjectPath
	State               uint32
	Interface           string
	ClonedAddress       string
	HwAddress           string
	Driver              string
	Managed             bool
	Vendor              string
	UniqueUuid          string
	UsbDevice           bool
	ActiveAp            dbus.ObjectPath
	SupportHotspot      bool
	Mode                uint32
	MobileNetworkType   string
	MobileSignalQuality uint32
	InterfaceFlags      uint32
}

func resetGSettings(gs *gio.Settings) {
	for _, key := range gs.ListKeys() {
		userVal := gs.GetUserValue(key)
		if userVal != nil {
			// TODO unref userVal
			logger.Debug("reset gsettings key", key)
			gs.Reset(key)
		}
	}
}

func resetKWin(wmObj wm.Wm) error {
	accels, err := util.GetAllKWinAccels(wmObj)
	if err != nil {
		return err
	}
	for _, accel := range accels {
		// logger.Debug("resetKwin each accel:", accel.Id, accel.Keystrokes, accel.DefaultKeystrokes)
		if !strv.Strv(accel.Keystrokes).Equal(accel.DefaultKeystrokes) &&
			len(accel.DefaultKeystrokes) > 0 && accel.DefaultKeystrokes[0] != "" {
			accelJson, err := util.MarshalJSON(&util.KWinAccel{
				Id:         accel.Id,
				Keystrokes: accel.DefaultKeystrokes,
			})
			if err != nil {
				logger.Warning(err)
				continue
			}
			logger.Debug("resetKwin SetAccel", accelJson)
			ok, err := wmObj.SetAccel(0, accelJson)
			// 目前 wm 的实现，调用 SetAccel 如果遇到冲突情况，会导致目标快捷键被清空。
			if !ok {
				logger.Warning("wm.SetAccel failed, id: ", accel.Id)
				continue
			}
			if err != nil {
				logger.Warning("failed to set accel:", err, accel.Id)
				continue
			}
		}
	}
	return nil
}

func showOSD(signal string) {
	logger.Debug("show OSD", signal)
	sessionDBus, _ := dbus.SessionBus()
	go sessionDBus.Object("com.deepin.dde.osd", "/").Call("com.deepin.dde.osd.ShowOSD", 0, signal)
}

const sessionManagerDest = "com.deepin.SessionManager"
const sessionManagerObjPath = "/com/deepin/SessionManager"

func systemLock() {
	sessionDBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return
	}
	go sessionDBus.Object(sessionManagerDest, sessionManagerObjPath).Call(sessionManagerDest+".RequestLock", 0)
}

func (m *Manager) canSuspend() bool {
	can, err := m.sessionManager.CanSuspend(0) // 当前能否待机
	if err != nil {
		logger.Warning(err)
		return false
	}
	return can
}

func (m *Manager) systemSuspend() {
	if !m.canSuspend() {
		logger.Info("can not suspend")
		return
	}

	logger.Debug("suspend")
	err := m.sessionManager.RequestSuspend(0)
	if err != nil {
		logger.Warning("failed to suspend:", err)
	}
}

// 为了处理待机闪屏的问题，通过前端进行待机，前端会在待机前显示一个纯黑的界面
func (m *Manager) systemSuspendByFront() {
	if !m.canExcuteSuspendOrHiberate || m.prepareForSleep {
		logger.Info("Avoid waking up and immediately going into suspend.")
		return
	}

	if !m.canSuspend() {
		logger.Info("can not suspend")
		return
	}

	logger.Debug("suspend")
	err := m.shutdownFront.Suspend(0)
	if err != nil {
		logger.Warning("failed to suspend:", err)
	}
}

func (m *Manager) canHibernate() bool {
	can, err := m.sessionManager.CanHibernate(0) // 能否休眠
	if err != nil {
		logger.Warning(err)
		return false
	}
	return can
}

func systemSuspend() {
	sessionDBus, _ := dbus.SessionBus()
	go sessionDBus.Object(sessionManagerDest, sessionManagerObjPath).Call(sessionManagerDest+".RequestSuspend", 0)
}

func (m *Manager) systemHibernate() {
	if !m.canHibernate() {
		logger.Info("can not Hibernate")
		return
	}

	logger.Debug("Hibernate")
	err := m.sessionManager.RequestHibernate(0)
	if err != nil {
		logger.Warning("failed to Hibernate:", err)
	}
}

func (m *Manager) systemHibernateByFront() {
	if !m.canExcuteSuspendOrHiberate || m.prepareForSleep {
		logger.Info("Avoid waking up and immediately going into suspend.")
		return
	}

	if !m.canHibernate() {
		logger.Info("can not Hibernate")
		return
	}

	logger.Debug("Hibernate")
	err := m.shutdownFront.Hibernate(0)
	if err != nil {
		logger.Warning("failed to Hibernate:", err)
	}
}

func (m *Manager) canShutdown() bool {
	can, err := m.sessionManager.CanShutdown(0) // 当前能否关机
	if err != nil {
		logger.Warning(err)
		return false
	}
	return can
}

func (m *Manager) hasShutdownInhibit() bool {
	// 先检查是否有delay 或 block shutdown的inhibitor
	inhibitors, err := m.login1Manager.ListInhibitors(0)
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

func (m *Manager) hasMultipleDisplaySession() bool {
	// 检查是否有多个图形session,有多个图形session就需要显示阻塞界面
	sessions, err := m.displayManager.Sessions().Get(0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return len(sessions) >= 2
}

func (m *Manager) systemShutdown() {
	// 如果是锁定状态，那么不需要后端进行关机响应，前端会显示关机或者关机阻塞界面；
	// 快捷键关机流程：判断是否存在多用户或任何shutdown阻塞项(block or delay),存在则跳转dde-shutdown界面，不存在进行sessionManager的注销
	if !m.canShutdown() {
		logger.Info("can not Shutdown")
	}

	locked, err := m.sessionManager.Locked().Get(0)
	if err != nil {
		logger.Warning("sessionManager get locked state error:", err)
		return
	}
	if locked {
		logger.Info("current session is locked")
		return
	}

	if m.hasShutdownInhibit() || m.hasMultipleDisplaySession() {
		logger.Info("exist shutdown inhibit(delay or block) or multiple display session")
		err := m.shutdownFront.Shutdown(0)
		if err != nil {
			logger.Warning(err)
		}
	} else {
		logger.Debug("keybinding start request SessionManager shutdown")
		err := m.sessionManager.RequestShutdown(0)
		if err != nil {
			logger.Warning("failed to Shutdown:", err)
		}
	}
}

func (m *Manager) systemTurnOffScreen() {
	const settingKeyScreenBlackLock = "screen-black-lock"
	logger.Info("DPMS Off")
	var err error
	var useWayland bool
	if len(os.Getenv("WAYLAND_DISPLAY")) != 0 {
		useWayland = true
	} else {
		useWayland = false
	}

	bScreenBlackLock := m.gsPower.GetBoolean(settingKeyScreenBlackLock)
	if bScreenBlackLock {
		m.doLock(true)
	}

	doPrepareSuspend()
	// bug-209669 : 部分厂商机器调用DPMS off后，调用锁屏show会被阻塞,只有当DPMS on了才show锁屏成功，这就导致唤醒闪桌面再进锁屏
	if bScreenBlackLock && !m.isWmBlackScreenActive() {
		m.setWmBlackScreenActive(true)
		time.Sleep(100 * time.Millisecond)
	}

	if useWayland {
		err = exec.Command("dde_wldpms", "-s", "Off").Run()
	} else {
		err = dpms.ForceLevelChecked(m.conn, dpms.DPMSModeOff).Check(m.conn)
	}
	if err != nil {
		logger.Warning("Set DPMS off error:", err)
	}

	if bScreenBlackLock && m.isWmBlackScreenActive() {
		m.setWmBlackScreenActive(false)
	}
	undoPrepareSuspend()
	ioutil.WriteFile("/tmp/dpms-state", []byte("1"), 0644)
}

func (m *Manager) systemLogout() {
	can, err := m.sessionManager.CanLogout(0)
	if err != nil {
		logger.Warning(err)
		return
	}

	if !can {
		logger.Info("can not logout")
		return
	}

	logger.Debug("logout")
	err = m.sessionManager.RequestLogout(0)
	if err != nil {
		logger.Warning("failed to logout:", err)
	}
}

func (m *Manager) systemAway() {
	err := m.sessionManager.RequestLock(0)
	if err != nil {
		logger.Warning(err)
	}
}

func queryCommandByMime(mime string) string {
	app := gio.AppInfoGetDefaultForType(mime, false)
	if app == nil {
		return ""
	}
	defer app.Unref()

	return app.GetExecutable()
}

func getRfkillWlanState() (int, error) {
	dir := "/sys/class/rfkill"
	fileInfoList, err := ioutil.ReadDir(dir)
	if err != nil {
		return 0, err
	}

	for _, fileInfo := range fileInfoList {
		typeFile := filepath.Join(dir, fileInfo.Name(), "type")
		typeBytes, err := readTinyFile(typeFile)
		if err != nil {
			continue
		}
		if bytes.Equal(bytes.TrimSpace(typeBytes), []byte("wlan")) {
			stateFile := filepath.Join(dir, fileInfo.Name(), "state")
			stateBytes, err := readTinyFile(stateFile)
			if err != nil {
				return 0, err
			}
			stateBytes = bytes.TrimSpace(stateBytes)
			state, err := strconv.Atoi(string(stateBytes))
			if err != nil {
				return 0, err
			}
			return state, nil

		}
	}
	return 0, errors.New("not found rfkill with type wlan")
}

func readTinyFile(file string) ([]byte, error) {
	f, err := os.Open(file)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	buf := make([]byte, 8)
	n, err := f.Read(buf)
	if err != nil {
		return nil, err
	}
	return buf[:n], nil
}

func shouldUseDDEKwin() bool {
	_, err := os.Stat("/usr/bin/kwin_no_scale")
	return err == nil
}

func (m *Manager) doLock(autoStartAuth bool) {
	logger.Info("Lock Screen")
	err := m.lockFront.ShowAuth(0, autoStartAuth)
	if err != nil {
		logger.Warning("failed to call lockFront ShowAuth:", err)
	}
}

func doPrepareSuspend() {
	sessionDBus, _ := dbus.SessionBus()
	obj := sessionDBus.Object("org.deepin.dde.Power1", "/org/deepin/dde/Power1")
	err := obj.Call("org.deepin.dde.Power1.SetPrepareSuspend", 0, suspendStateButtonClick).Err
	if err != nil {
		logger.Warning(err)
	}
}

func undoPrepareSuspend() {
	sessionDBus, _ := dbus.SessionBus()
	obj := sessionDBus.Object("org.deepin.dde.Power1", "/org/deepin/dde/Power1")
	err := obj.Call("org.deepin.dde.Power.SetPrepareSuspend", 0, suspendStateFinish).Err
	if err != nil {
		logger.Warning(err)
	}
}
