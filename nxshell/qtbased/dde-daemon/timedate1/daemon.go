// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedate

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var (
	logger = log.NewLogger("daemon/timedate")
)

type Daemon struct {
	*loader.ModuleBase
	manager       *Manager
	managerFormat *ManagerFormat
}

func NewDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("timedate", daemon, logger)
	return daemon
}

func (d *Daemon) GetDependencies() []string {
	return []string{}
}

// Start to run time date manager
func (d *Daemon) Start() error {
	if d.manager != nil {
		return nil
	}
	service := loader.GetService()

	var err error
	d.manager, err = NewManager(service)
	if err != nil {
		return err
	}

	err = service.Export(dbusPath, d.manager)
	if err != nil {
		return err
	}

	d.managerFormat, err = newManagerFormat(service)
	if err != nil {
		return err
	}

	err = service.Export(dbusFormatPath, d.managerFormat)
	if err != nil {
		return err
	}

	err = d.managerFormat.initPropertyWriteCallback(service)
	if err != nil {
		logger.Warning("call SetWriteCallback err:", err)
	} else {
		logger.Info("call SetWriteCallback successful.")
	}

	err = service.RequestName(dbusFormatServiceName)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	go func() {
		d.manager.init()
	}()

	go func() {
		d.managerFormat.init()
	}()

	return nil
}

// Stop the time date manager
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

	d.managerFormat.destroy()
	d.managerFormat = nil
	return nil
}
