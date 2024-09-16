// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package screenedge

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var (
	logger = log.NewLogger("daemon/screenedge")
)

type Daemon struct {
	*loader.ModuleBase
	manager *Manager
}

func NewDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("screenedge", daemon, logger)
	return daemon
}

func (d *Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {
	if d.manager != nil {
		return nil
	}
	service := loader.GetService()
	d.manager = newManager(service)

	err := service.Export(dbusPath, d.manager, d.manager.syncConfig)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	err = d.manager.syncConfig.Register()
	if err != nil {
		logger.Warning(err)
	}
	return nil
}

func (d *Daemon) Stop() error {
	if d.manager == nil {
		return nil
	}
	service := loader.GetService()
	err := service.StopExport(d.manager)
	if err != nil {
		return err
	}

	err = service.ReleaseName(dbusServiceName)
	if err != nil {
		return err
	}

	d.manager.destroy()
	d.manager = nil
	return nil
}
