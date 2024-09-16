// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/dde-daemon/loader"
)

var logger = log.NewLogger("daemon/launcher")

func init() {
	loader.Register(NewModule(logger))
}

type Module struct {
	*loader.ModuleBase
	manager *Manager
}

func NewModule(logger *log.Logger) *Module {
	daemon := new(Module)
	daemon.ModuleBase = loader.NewModuleBase("launcher", daemon, logger)
	return daemon
}

func (d *Module) GetDependencies() []string {
	return []string{}
}

func (d *Module) start() error {
	service := loader.GetService()

	var err error
	d.manager, err = NewManager(service)
	if err != nil {
		return err
	}

	err = service.Export(dbusObjPath, d.manager, d.manager.syncConfig)
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

func (d *Module) Start() error {
	return d.start()
}

func (d *Module) Stop() error {
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

	d.manager.destroy()
	d.manager = nil
	return nil
}
