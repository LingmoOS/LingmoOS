// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices1

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

var logger = log.NewLogger("daemon/system/inputdevices")

const (
	dbusServiceName = "org.deepin.dde.InputDevices1"
	dbusPath        = "/org/deepin/dde/InputDevices1"
	dbusInterface   = dbusServiceName
)

func init() {
	loader.Register(newDaemon())
}

type daemon struct {
	*loader.ModuleBase
	inputdevices *InputDevices
}

func newDaemon() *daemon {
	d := new(daemon)
	d.ModuleBase = loader.NewModuleBase("inputdevices", d, logger)
	return d
}

func (d *daemon) GetDependencies() []string {
	return []string{}
}

func (d *daemon) Start() error {
	if d.inputdevices != nil {
		return nil
	}

	logger.Debug("start inputdevices")
	d.inputdevices = newInputDevices()

	service := loader.GetService()
	d.inputdevices.service = service
	d.inputdevices.systemSigLoop = dbusutil.NewSignalLoop(service.Conn(), 5)
	d.inputdevices.init()

	err := service.Export(dbusPath, d.inputdevices)
	if err != nil {
		logger.Warning(err)
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		logger.Warning(err)
		return err
	}

	return nil
}

func (d *daemon) Stop() error {
	return nil
}
