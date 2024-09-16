// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedated

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

type Daemon struct {
	*loader.ModuleBase
}

var (
	logger   = log.NewLogger("timedated")
	_manager *Manager
)

func NewDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("timedated", daemon, logger)
	return daemon
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}

func init() {
	loader.Register(NewDaemon(logger))
}

func (d *Daemon) Start() error {
	if _manager != nil {
		return nil
	}

	var err error
	service := loader.GetService()
	_manager, err = NewManager(service)
	if err != nil {
		logger.Error("Failed to new timedated manager:", err)
		return err
	}

	err = service.Export(dbusPath, _manager)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	_manager.start()
	return nil
}

func (*Daemon) Stop() error {
	if _manager != nil {
		_manager.destroy()
		_manager = nil
	}

	return nil
}
