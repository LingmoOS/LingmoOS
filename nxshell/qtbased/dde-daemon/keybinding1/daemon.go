// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

func init() {
	loader.Register(NewDaemon(logger))
	shortcuts.SetLogger(logger)
}

type Daemon struct {
	*loader.ModuleBase
	manager *Manager
}

var (
	logger = log.NewLogger("daemon/keybinding")
)

func NewDaemon(logger *log.Logger) *Daemon {
	var d = new(Daemon)
	d.ModuleBase = loader.NewModuleBase("keybinding", d, logger)
	return d
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {
	if d.manager != nil {
		return nil
	}
	var err error

	service := loader.GetService()

	loadConfig()

	d.manager, err = newManager(service)
	if err != nil {
		return err
	}

	err = service.Export(dbusPath, d.manager)
	if err != nil {
		d.manager.destroy()
		d.manager = nil
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		d.manager.destroy()
		d.manager = nil
		return err
	}

	go func() {
		m := d.manager
		m.initHandlers()

		// listen gsettings changed event
		m.listenGSettingsChanged(gsSchemaSystem, d.manager.gsSystem, shortcuts.ShortcutTypeSystem)
		m.listenGSettingsChanged(gsSchemaMediaKey, d.manager.gsMediaKey, shortcuts.ShortcutTypeMedia)
		m.listenGSettingsChanged(gsSchemaGnomeWM, d.manager.gsGnomeWM, shortcuts.ShortcutTypeWM)

		m.listenSystemEnableChanged()
		m.listenSystemPlatformChanged()

		m.eliminateKeystrokeConflict()
		m.shortcutManager.EventLoop()
	}()

	return nil
}

func (d *Daemon) Stop() error {
	if d.manager == nil {
		return nil
	}

	service := loader.GetService()
	err := service.ReleaseName(dbusServiceName)
	if err != nil {
		logger.Warning(err)
	}

	d.manager.destroy()
	d.manager = nil
	return nil
}
