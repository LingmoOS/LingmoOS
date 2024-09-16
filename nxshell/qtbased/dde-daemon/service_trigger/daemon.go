// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package service_trigger

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

func init() {
	loader.Register(NewDaemon())
}

var logger = log.NewLogger("daemon/" + moduleName)

const moduleName = "service-trigger"

type Daemon struct {
	*loader.ModuleBase
	manager *Manager
}

func NewDaemon() *Daemon {
	d := &Daemon{}
	d.ModuleBase = loader.NewModuleBase(moduleName, d, logger)
	return d
}

func (d *Daemon) Start() error {
	service := loader.GetService()
	m := newManager(service)
	err := m.start()
	if err != nil {
		return err
	}
	d.manager = m
	return nil
}

func (d *Daemon) Stop() error {
	if d.manager != nil {
		err := d.manager.stop()
		if err != nil {
			return err
		}
		d.manager = nil
	}
	return nil
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}
