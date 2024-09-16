// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package sessionwatcher

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var (
	logger = log.NewLogger("daemon/sessionwatcher")
)

type Daemon struct {
	*loader.ModuleBase
	manager *Manager
}

func init() {
	loader.Register(NewDaemon(logger))
}

func NewDaemon(logger *log.Logger) *Daemon {
	var d = new(Daemon)
	d.ModuleBase = loader.NewModuleBase("sessionwatcher", d, logger)
	return d
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {
	if d.manager != nil {
		return nil
	}
	service := loader.GetService()

	var err error
	d.manager, err = newManager(service)
	if err != nil {
		return err
	}

	d.manager.initUserSessions()

	err = service.Export(dbusPath, d.manager)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
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
		logger.Warning("StopExport error:", err)
	}
	d.manager.destroy()
	d.manager = nil
	return nil
}
