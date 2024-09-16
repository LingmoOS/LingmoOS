// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"github.com/linuxdeepin/dde-daemon/network1"
	"syscall"

	"github.com/godbus/dbus/v5"
	daemon "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.daemon1"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

type sleepInhibitor struct {
	loginManager      login1.Manager
	fd                int
	dbusObj           ofdbus.DBus
	sysLoop           *dbusutil.SignalLoop
	hasRunBeforeSleep bool

	OnWakeup        func()
	OnBeforeSuspend func()
}

func newSleepInhibitor(login1Manager login1.Manager, daemon daemon.Daemon) *sleepInhibitor {
	inhibitor := &sleepInhibitor{
		loginManager: login1Manager,
		fd:           -1,
	}
	systemBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning(err)
		return nil
	}
	inhibitor.dbusObj = ofdbus.NewDBus(systemBus)
	inhibitor.sysLoop = dbusutil.NewSignalLoop(systemBus, 10)
	inhibitor.sysLoop.Start()
	inhibitor.dbusObj.InitSignalExt(inhibitor.sysLoop, true)
	_, err = daemon.ConnectHandleForSleep(func(before bool) {
		logger.Info("sleepInhibitor HandleForSleep", before)
		// signal `HandleForSleep` true -> false
		if before {
			_manager.PropsMu.Lock()
			if !_manager.sessionActive {
				logger.Debug("sessionActive is false,don't need run before sleep")
				_manager.PropsMu.Unlock()
				// 如果此用户此时不是活跃状态,则不处理待机唤醒信号.
				return
			}
			_manager.PropsMu.Unlock()

			inhibitor.hasRunBeforeSleep = true
			// TODO(jouyouyun): implement 'HandleForSleep' register
			//appearance.HandlePrepareForSleep(true)
			network.HandlePrepareForSleep(true)
			if inhibitor.OnBeforeSuspend != nil {
				inhibitor.OnBeforeSuspend()
			}
			suspendPulseSinks(1)
			suspendPulseSources(1)
			err := inhibitor.unblock()
			if err != nil {
				logger.Warning(err)
			}
		} else {
			if !inhibitor.hasRunBeforeSleep {
				logger.Debug("not run before sleep,don't need run after sleep")
				return
			}
			suspendPulseSources(0)
			suspendPulseSinks(0)
			if inhibitor.OnWakeup != nil {
				inhibitor.OnWakeup()
			}
			if _manager != nil {
				_manager.handleRefreshMains()
				_manager.handleBatteryDisplayUpdate()
				_manager.handleWakeupDDELowPowerCheck()
			}
			network.HandlePrepareForSleep(false)
			//appearance.HandlePrepareForSleep(false)
			err := inhibitor.block()
			if err != nil {
				logger.Warning(err)
			}
			inhibitor.hasRunBeforeSleep = false
		}
	})
	if err != nil {
		logger.Warning(err)
	}
	return inhibitor
}

func (inhibitor *sleepInhibitor) block() error {
	logger.Debug("block sleep")
	if inhibitor.fd != -1 {
		return nil
	}
	inhibit := func() {
		fd, err := inhibitor.loginManager.Inhibit(0,
			"sleep", dbusServiceName, "run screen lock", "delay")
		if err != nil {
			logger.Warning(err)
			return
		}
		inhibitor.fd = int(fd)
	}
	inhibit()
	// handle login1 restart
	_, _ = inhibitor.dbusObj.ConnectNameOwnerChanged(func(name string, oldOwner string, newOwner string) {
		if name == "org.freedesktop.login1" && newOwner != "" && oldOwner == "" {
			if inhibitor.fd != -1 { // 如果之前存在inhibit时，login1重启需要重新inhibit
				err := syscall.Close(inhibitor.fd)
				inhibitor.fd = -1
				if err != nil {
					logger.Warning("failed to close fd:", err)
					return
				}
				inhibit()
			}
		}
	})
	// end handle login1 restart
	return nil
}

func (inhibitor *sleepInhibitor) unblock() error {
	if inhibitor.fd == -1 {
		return nil
	}
	logger.Debug("unblock sleep")
	err := syscall.Close(inhibitor.fd)
	inhibitor.fd = -1
	if err != nil {
		logger.Warning("failed to close fd:", err)
		return err
	}
	inhibitor.dbusObj.RemoveAllHandlers()
	return nil
}
