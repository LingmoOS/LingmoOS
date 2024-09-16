// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"os/exec"
	"strings"
	"time"
)

func init() {
	submoduleList = append(submoduleList, newLidSwitchHandler)
}

// nolint
const (
	lidSwitchStateUnknown = iota
	lidSwitchStateOpen
	lidSwitchStateClose
)

type LidSwitchHandler struct {
	manager *Manager
	cmd     *exec.Cmd
	cookie  chan struct{}
}

func newLidSwitchHandler(m *Manager) (string, submodule, error) {
	h := &LidSwitchHandler{
		manager: m,
	}
	return "LidSwitchHandler", h, nil
}

func (h *LidSwitchHandler) Start() error {
	power := h.manager.helper.Power
	_, err := power.ConnectLidClosed(h.onLidClosed)
	if err != nil {
		return err
	}
	_, err = power.ConnectLidOpened(h.onLidOpened)
	if err != nil {
		return err
	}
	return nil
}

func (h *LidSwitchHandler) onLidClosed() {
	h.onLidDelayOperate(false)
}

func (h *LidSwitchHandler) onLidOpened() {
	h.onLidDelayOperate(true)
}

func (h *LidSwitchHandler) onLidDelayOperate(state bool) {
	if _manager != nil && !_manager.sessionActive { // session未激活状态不处理
		return
	}
	if h.cookie != nil {
		h.cookie <- struct{}{}
		close(h.cookie)
	}

	h.cookie = make(chan struct{}, 1)

	go func() {
		select {
		case <-h.cookie:
			break
		// 1.5s是为了保证在关闭显示器时，其强制等待的1秒操作被完全执行，否则会带来闪屏问题
		case <-time.After(time.Millisecond * 1500):
			h.doLidStateChanged(state)
			break
		}
	}()
}

func (h *LidSwitchHandler) doLidStateChanged(state bool) {
	logger.Info("Lid open:", state)

	m := h.manager
	m.setPrepareSuspend(suspendStateLidClose)
	m.PropsMu.Lock()
	m.lidSwitchState = lidSwitchStateClose
	m.PropsMu.Unlock()
	m.claimOrReleaseAmbientLight()

	// 合盖
	if !state {
		var onBattery bool
		onBattery = h.manager.OnBattery
		var lidCloseAction int32
		if onBattery {
			lidCloseAction = m.BatteryLidClosedAction.Get() // 获取合盖操作
		} else {
			lidCloseAction = m.LinePowerLidClosedAction.Get() // 获取合盖操作
		}
		switch lidCloseAction {
		case powerActionShutdown:
			m.doShutdown()
		case powerActionSuspend:
			m.doSuspendByFront()
		case powerActionHibernate:
			m.doHibernateByFront()
		case powerActionTurnOffScreen:
			m.doTurnOffScreen()
		case powerActionDoNothing:
			return
		}

		if lidCloseAction != powerActionTurnOffScreen {
			m.setDDEBlackScreenActive(true)
		}
	} else { // 开盖
		err := h.stopAskUser()
		if err != nil {
			logger.Warning("stopAskUser error:", err)
		}

		// x11环境在待机唤醒后，会接收到screensaver的ConnectIdleOff信息将待机状态更新为suspendStateFinish状态
		// wayland环境下，并没有做这个处理，此处在开盖后手动更新状态为suspendStateFinish
		if m.UseWayland {
			m.setPrepareSuspend(suspendStateFinish)
		}

		err = m.helper.ScreenSaver.SimulateUserActivity(0)
		if err != nil {
			logger.Warning(err)
		}

		m.setDDEBlackScreenActive(false)
		m.setDPMSModeOn()
	}
}

func (h *LidSwitchHandler) stopAskUser() error {
	if h.cmd == nil {
		return nil
	}

	if h.cmd.ProcessState == nil {
		// h.cmd is running
		logger.Debug("stopAskUser: kill process")
		err := h.cmd.Process.Kill()
		if err != nil {
			return err
		}
	} else {
		logger.Debug("stopAskUser: cmd exited")
	}
	h.cmd = nil
	return nil
}

// copy from display module of project startdde
func isBuiltinOutput(name string) bool {
	name = strings.ToLower(name)
	switch {
	case strings.Contains(name, "lvds"):
		// Most drivers use an "LVDS" prefix
		fallthrough
	case strings.Contains(name, "lcd"):
		// fglrx uses "LCD" in some versions
		fallthrough
	case strings.Contains(name, "edp"):
		// eDP is for internal built-in panel connections
		fallthrough
	case strings.Contains(name, "dsi"):
		return true
	case name == "default":
		// now sunway notebook has only one output named default
		return true
	}
	return false
}

func (h *LidSwitchHandler) Destroy() {
}
