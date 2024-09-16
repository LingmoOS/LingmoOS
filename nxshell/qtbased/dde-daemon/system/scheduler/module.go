// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package scheduler

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var logger = log.NewLogger("daemon/system/scheduler")

func init() {
	loader.Register(newModule(logger))
}

type Module struct {
	*loader.ModuleBase
}

func (m *Module) GetDependencies() []string {
	return nil
}

func (m *Module) Start() error {
	err := start()
	return err
}

func (m *Module) Stop() error {
	return nil
}

func newModule(logger *log.Logger) *Module {
	m := new(Module)
	m.ModuleBase = loader.NewModuleBase("scheduler", m, logger)
	return m
}
