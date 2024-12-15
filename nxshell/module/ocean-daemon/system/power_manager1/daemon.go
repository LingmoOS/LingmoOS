// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power_manager

import (
	"github.com/lingmoos/ocean-daemon/loader"
	"github.com/lingmoos/go-lib/log"
)

const (
	dbusServiceName = "org.lingmo.ocean.PowerManager1"
	dbusPath        = "/org/lingmo/ocean/PowerManager1"
	dbusInterface   = dbusServiceName
)

var logger = log.NewLogger("daemon/system/powermanager")

func init() {
	loader.Register(NewDaemon(logger))
}

type Daemon struct {
	*loader.ModuleBase
	manager *Manager
}

func NewDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("powermanager", daemon, logger)
	return daemon
}

func (d *Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() (err error) {
	service := loader.GetService()
	d.manager, err = newManager(service)
	if err != nil {
		return
	}

	err = service.Export(dbusPath, d.manager)
	if err != nil {
		return
	}

	err = service.RequestName(dbusServiceName)
	return
}

func (d *Daemon) Stop() error {
	if d.manager == nil {
		return nil
	}

	d.manager = nil
	return nil
}
