// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

//go:generate dbusutil-gen -type Keyboard,Mouse,Touchpad,TrackPoint,Wacom keyboard.go mouse.go touchpad.go trackpoint.go wacom.go
//go:generate dbusutil-gen em -type Keyboard,Mouse,Touchpad,TrackPoint,Wacom,Manager

var (
	_manager *Manager
	logger   = log.NewLogger("daemon/inputdevices")
)

type Daemon struct {
	*loader.ModuleBase
}

func init() {
	loader.Register(NewInputdevicesDaemon(logger))
}
func NewInputdevicesDaemon(logger *log.Logger) *Daemon {
	var d = new(Daemon)
	d.ModuleBase = loader.NewModuleBase("inputdevices", d, logger)
	return d
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {
	if _manager != nil {
		return nil
	}

	service := loader.GetService()
	_manager = NewManager(service)

	err := service.Export(dbusPath, _manager, _manager.syncConfig)
	if err != nil {
		return err
	}

	err = service.Export(kbdDBusPath, _manager.kbd)
	if err != nil {
		return err
	}

	kbdServerObj := service.GetServerObject(_manager.kbd)
	err = kbdServerObj.SetWriteCallback(_manager.kbd, "CurrentLayout",
		_manager.kbd.setCurrentLayout)
	if err != nil {
		return err
	}

	err = service.Export(wacomDBusPath, _manager.wacom)
	if err != nil {
		return err
	}

	err = service.Export(touchPadDBusPath, _manager.tpad)
	if err != nil {
		return err
	}

	err = service.Export(mouseDBusPath, _manager.mouse, _manager.trackPoint)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	go func() {
		_manager.init()
		err := _manager.syncConfig.Register()
		if err != nil {
			logger.Warning(err)
		}
		if globalWayland {
			_manager.handleInputDeviceChanged(false)
			return
		}
		startDeviceListener()
	}()
	return nil
}

func (*Daemon) Stop() error {
	if _manager == nil {
		return nil
	}

	_manager.sessionSigLoop.Stop()
	_manager.syncConfig.Destroy()

	if _manager.kbd != nil {
		_manager.kbd.destroy()
		_manager.kbd = nil
	}

	if _manager.wacom != nil {
		_manager.wacom.destroy()
		_manager.wacom = nil
	}

	if _manager.tpad != nil {
		_manager.tpad.destroy()
		_manager.tpad = nil
	}
	_manager = nil

	if globalWayland {
		_manager.handleInputDeviceChanged(true)
		return nil
	}
	// TODO endDeviceListener will be stuck
	endDeviceListener()
	return nil
}
