// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package uadpagent

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

func init() {
	loader.Register(NewModule(logger))
}

type Module struct {
	uAgent *UadpAgent
	*loader.ModuleBase
}

func NewModule(logger *log.Logger) *Module {
	m := new(Module)
	m.ModuleBase = loader.NewModuleBase("uadpagent", m, logger)
	return m
}

func (m *Module) GetDependencies() []string {
	return []string{}
}

func (m *Module) Start() error {
	// service := loader.GetService()

	// if m.uAgent != nil {
	// 	return nil
	// }

	// var err error
	// m.uAgent, err = newUadpAgent(service)
	// if err != nil {
	// 	logger.Warning("failed to newUadpAgent:", err)
	// 	return err
	// }

	// err = service.Export(dbusPath, m.uAgent)
	// if err != nil {
	// 	logger.Warning("failed to Export uAgent:", err)
	// 	return err
	// }

	// err = service.RequestName(dbusServiceName)
	// if err != nil {
	// 	logger.Warning("failed to RequestName:", err)
	// 	return err
	// }

	return nil
}

func (m *Module) Stop() error {
	if m.uAgent == nil {
		return nil
	}

	service := loader.GetService()
	err := service.ReleaseName(dbusServiceName)
	if err != nil {
		logger.Warning("failed to releaseName:", err)
	}

	err = service.StopExport(m.uAgent)
	if err != nil {
		logger.Warning("failed to stopExport:", err)
	}
	m.uAgent = nil

	return nil
}
