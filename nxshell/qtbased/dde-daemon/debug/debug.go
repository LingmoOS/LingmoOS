// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package debug

import (
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var (
	logger = log.NewLogger("daemon/debug")
)

type Daemon struct {
	*loader.ModuleBase
}

func NewDaemon() *Daemon {
	var d = new(Daemon)
	d.ModuleBase = loader.NewModuleBase("debug", d, logger)
	return d
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {
	if d.LogLevel() != log.LevelDebug {
		loader.ToggleLogDebug(true)
	}

	return nil
}

func (d *Daemon) Stop() error {
	if d.LogLevel() == log.LevelDebug {
		loader.ToggleLogDebug(false)
	}
	return nil
}
