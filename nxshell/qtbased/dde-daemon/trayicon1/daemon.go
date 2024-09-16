// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package trayicon

import (
	"os"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	x "github.com/linuxdeepin/go-x11-client"
)

type Daemon struct {
	*loader.ModuleBase
	manager *TrayManager
	snw     *StatusNotifierWatcher
	sigLoop *dbusutil.SignalLoop // session bus signal loop
}

const moduleName = "trayicon"

func NewDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase(moduleName, daemon, logger)
	return daemon
}

func (d *Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Name() string {
	return moduleName
}

func (d *Daemon) Start() error {
	var err error
	// init x conn
	XConn, err = x.NewConn()
	if err != nil {
		return err
	}

	initX()
	service := loader.GetService()
	d.manager = NewTrayManager(service)

	sessionBus, err := dbus.SessionBus()
	if err != nil {
		return err
	}

	d.sigLoop = dbusutil.NewSignalLoop(sessionBus, 10)
	d.sigLoop.Start()

	err = service.Export(dbusPath, d.manager)
	if err != nil {
		return err
	}

	err = d.manager.sendClientMsgMANAGER()
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	err = service.Emit(d.manager, "Inited")
	if err != nil {
		return err
	}

	if os.Getenv("DDE_DISABLE_STATUS_NOTIFIER_WATCHER") != "1" {
		d.snw = newStatusNotifierWatcher(service, d.sigLoop)
		d.snw.listenDBusNameOwnerChanged()
		err = service.Export(snwDBusPath, d.snw)
		if err != nil {
			return err
		}
		err = service.RequestName(snwDBusServiceName)
		if err != nil {
			logger.Warning("failed to request name:", err)
			return nil
		}
	} else {
		logger.Info("disable status notifier watcher")
	}
	return nil
}

func (d *Daemon) Stop() error {
	if XConn != nil {
		XConn.Close()
	}
	return nil
}
