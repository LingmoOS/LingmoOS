// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package airplane_mode

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var logger *log.Logger

func init() {
	logger = log.NewLogger("daemon/airplane_mode")
	loader.Register(NewModule())
}

type Module struct {
	m *Manager
	*loader.ModuleBase
}

func (m *Module) GetDependencies() []string {
	return nil
}

func (m *Module) Start() error {
	if m.m != nil {
		return nil
	}
	logger.Debug("airplane mode module start")
	service := loader.GetService()
	m.m = newManager(service)
	err := service.Export(dbusPath, m.m)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}
	return nil
}

func (m *Module) Stop() error {
	return nil
}

func NewModule() *Module {
	m := &Module{}
	m.ModuleBase = loader.NewModuleBase("airplane_mode", m, logger)
	return m
}
