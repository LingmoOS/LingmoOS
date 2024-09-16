// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package fprintd1

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var (
	logger = log.NewLogger("daemon/fprintd")
)

type Daemon struct {
	*loader.ModuleBase
	manager *Manager
}

func NewDaemon() *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("fprintd", daemon, logger)
	return daemon
}

func init() {
	loader.Register(NewDaemon())
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

	err = service.Export(dbusPath, d.manager)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	go d.manager.init()
	return nil
}

func (d *Daemon) Stop() error {
	if d.manager == nil {
		return nil
	}

	service := loader.GetService()

	err := service.ReleaseName(dbusServiceName)
	if err != nil {
		return err
	}

	err = service.StopExport(d.manager)
	if err != nil {
		return err
	}

	d.manager.destroy()
	d.manager = nil
	return nil
}
