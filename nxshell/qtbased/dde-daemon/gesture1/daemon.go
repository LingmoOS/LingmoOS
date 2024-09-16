// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package gesture1

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

type Daemon struct {
	*loader.ModuleBase
	manager *Manager
}

const (
	dbusServiceName = "org.deepin.dde.Gesture1"
	dbusServicePath = "/org/deepin/dde/Gesture1"
	dbusServiceIFC  = dbusServiceName
)

var (
	logger = log.NewLogger("gesture")
)

func NewDaemon() *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("gesture", daemon, logger)
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

	var err error
	d.manager, err = newManager()
	if err != nil {
		logger.Error("failed to initialize gesture manager:", err)
		return err
	}

	service := loader.GetService()
	err = service.Export(dbusServicePath, d.manager)
	if err != nil {
		logger.Error("failed to export gesture:", err)
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		logger.Error("failed to request gesture name:", err)
		d.manager.destroy()
		err1 := service.StopExport(d.manager)
		if err1 != nil {
			logger.Error("failed to StopExport:", err1)
		}
		return err
	}

	d.manager.init()

	return nil
}

func (d *Daemon) Stop() error {
	if xconn != nil {
		xconn.Close()
		xconn = nil
	}

	if d.manager == nil {
		return nil
	}

	d.manager.destroy()
	d.manager = nil
	return nil
}
