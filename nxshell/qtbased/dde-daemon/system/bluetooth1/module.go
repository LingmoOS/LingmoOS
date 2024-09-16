// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"github.com/linuxdeepin/dde-daemon/common/sessionmsg"
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

type module struct {
	*loader.ModuleBase
}

func newBluetoothModule(logger *log.Logger) *module {
	var d = new(module)
	d.ModuleBase = loader.NewModuleBase("bluetooth", d, logger)
	return d
}

func (*module) GetDependencies() []string {
	return nil
}

var _bt *SysBluetooth

func (m *module) Start() error {
	if _bt != nil {
		return nil
	}

	service := loader.GetService()
	_bt = newSysBluetooth(service)

	sessionmsg.SetAgentInfoPublisher(_bt.userAgents)

	err := service.Export(dbusPath, _bt)
	if err != nil {
		logger.Warning("failed to export bluetooth:", err)
		_bt = nil
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	_bt.agent = newAgent(service)
	_bt.agent.b = _bt

	err = service.Export(agentDBusPath, _bt.agent)
	if err != nil {
		logger.Warning("failed to export agent:", err)
		return err
	}

	// initialize bluetooth after dbus interface installed
	go _bt.init()
	return nil
}

func (*module) Stop() error {
	if _bt == nil {
		return nil
	}

	sessionmsg.SetAgentInfoPublisher(nil)

	service := loader.GetService()
	err := service.ReleaseName(dbusServiceName)
	if err != nil {
		logger.Warning(err)
	}

	_bt.destroy()
	_bt = nil
	return nil
}
