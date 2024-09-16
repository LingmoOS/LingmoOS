// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package x_event_monitor

import (
	"os"
	"strings"

	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

const (
	dbusServiceName = "org.deepin.dde.XEventMonitor1"
	dbusPath        = "/org/deepin/dde/XEventMonitor1"
	dbusInterface   = dbusServiceName
	moduleName      = "x-event-monitor"
)

var (
	logger = log.NewLogger(moduleName)
)

func init() {
	loader.Register(NewDaemon(logger))
}

type Daemon struct {
	*loader.ModuleBase
}

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
	service := loader.GetService()

	m, err := newManager(service)
	if err != nil {
		return err
	}
	m.initXExtensions()

	sessionType := os.Getenv("XDG_SESSION_TYPE")
	if strings.Contains(sessionType, "wayland") {
		go m.listenGlobalCursorPressed()
		go m.listenGlobalCursorRelease()
		go m.listenGlobalCursorMove()
		go m.listenGlobalAxisChanged()
	} else {
		go m.handleXEvent()
	}

	err = service.Export(dbusPath, m)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	err = service.Emit(m, "CancelAllArea")
	if err != nil {
		logger.Warning("Emit error:", err)
	}

	return nil
}

func (d *Daemon) Stop() error {
	return nil
}
