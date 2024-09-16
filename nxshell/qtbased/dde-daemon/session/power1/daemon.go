// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var logger = log.NewLogger("daemon/session/power")

func init() {
	loader.Register(NewDaemon(logger))
}

type Daemon struct {
	*loader.ModuleBase
	manager *Manager
}

func NewDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("power", daemon, logger)
	return daemon
}

func (d *Daemon) GetDependencies() []string {
	return []string{"screensaver", "sessionwatcher"}
}

func (d *Daemon) Start() error {
	service := loader.GetService()
	var err error
	d.manager, err = newManager(service)
	if err != nil {
		return err
	}

	err = service.Export(dbusPath, d.manager,
		d.manager.warnLevelConfig, d.manager.syncConfig)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	err = d.manager.syncConfig.Register()
	if err != nil {
		logger.Warning("failed to register for deepin sync:", err)
	}

	go d.manager.init()
	_manager = d.manager
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
	err = service.StopExport(d.manager)
	if err != nil {
		logger.Warning(err)
	}
	err = service.StopExport(d.manager.syncConfig)
	if err != nil {
		logger.Warning(err)
	}

	d.manager.destroy()
	d.manager = nil
	return nil
}
