// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub_gfx

import (
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/dde-daemon/loader"
)

const moduleName = "grub-gfx"

var logger = log.NewLogger(moduleName)

type module struct {
	*loader.ModuleBase
}

func (*module) GetDependencies() []string {
	return nil
}

func (d *module) Start() error {
	detectChange()
	return nil
}

func (d *module) Stop() error {
	return nil
}

func newModule() *module {
	d := new(module)
	d.ModuleBase = loader.NewModuleBase(moduleName, d, logger)
	return d
}

func init() {
	loader.Register(newModule())
}
