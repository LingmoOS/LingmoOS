// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package screensaver

import (
	"github.com/linuxdeepin/dde-daemon/common/dsync"
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

func init() {
	loader.Register(newModule(logger))
}

type Module struct {
	sSaver     *ScreenSaver
	syncConfig *dsync.Config
	*loader.ModuleBase
}

func newModule(logger *log.Logger) *Module {
	m := new(Module)
	m.ModuleBase = loader.NewModuleBase("screensaver", m, logger)
	return m
}

func (m *Module) GetDependencies() []string {
	return []string{}
}

func (m *Module) Start() error {
	service := loader.GetService()

	has, err := service.NameHasOwner(dbusServiceName)
	if err != nil {
		return err
	}
	if has {
		logger.Warning("ScreenSaver has been register, exit...")
		return nil
	}

	if m.sSaver != nil {
		return nil
	}

	m.sSaver, err = newScreenSaver(service)
	if err != nil {
		return err
	}

	err = service.Export(dbusPath, m.sSaver)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	m.syncConfig = dsync.NewConfig("screensaver", &syncConfig{}, m.sSaver.sigLoop, dScreenSaverPath, logger)
	err = service.Export(dScreenSaverPath, m.syncConfig)
	if err != nil {
		return err
	}

	err = service.RequestName(dScreenSaverServiceName)
	if err != nil {
		return err
	}

	err = m.syncConfig.Register()
	if err != nil {
		logger.Warning("failed to register for deepin sync:", err)
	}

	return nil
}

func (m *Module) Stop() error {
	if m.sSaver == nil {
		return nil
	}

	service := loader.GetService()
	err := service.ReleaseName(dbusServiceName)
	if err != nil {
		logger.Warning(err)
	}

	err = service.StopExport(m.sSaver)
	if err != nil {
		logger.Warning(err)
	}
	m.sSaver.destroy()
	m.sSaver = nil

	err = service.ReleaseName(dScreenSaverServiceName)
	if err != nil {
		logger.Warning(err)
	}

	err = service.StopExport(m.syncConfig)
	if err != nil {
		logger.Warning(err)
	}
	m.syncConfig.Destroy()
	if m.sSaver.xConn != nil {
		m.sSaver.xConn.Close()
	}
	return nil
}
