// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package apps1

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

//go:generate dbusutil-gen em -type ALRecorder,DFWatcher

var logger = log.NewLogger("daemon/apps")

func init() {
	loader.Register(NewDaemon(logger))
}

type Daemon struct {
	*loader.ModuleBase
	recorder *ALRecorder
	watcher  *DFWatcher
}

func NewDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("apps", daemon, logger)
	return daemon
}

func (d *Daemon) Start() error {
	service := loader.GetService()

	var err error
	d.watcher, err = newDFWatcher(service)
	if err != nil {
		return err
	}

	d.recorder, err = newALRecorder(d.watcher)
	if err != nil {
		return err
	}

	// export recorder and watcher
	err = service.Export(dbusPath, d.recorder, d.watcher)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}
	d.recorder.emitServiceRestarted()

	return nil
}

func (d *Daemon) Stop() error {
	// TODO
	return nil
}

func (d *Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Name() string {
	return "apps"
}
