// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package clipboard

import (
	"os"

	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/ext/xfixes"
)

const (
	dbusServiceName = "org.deepin.dde.ClipboardManager1"
	dbusPath        = "/org/deepin/dde/ClipboardManager1"
)

var logger *log.Logger

func init() {
	logger = log.NewLogger("clipboard")
	loader.Register(newModule())
}

func newModule() *Module {
	m := new(Module)
	m.ModuleBase = loader.NewModuleBase("clipboard", m, logger)
	return m
}

type Module struct {
	*loader.ModuleBase
}

func (*Module) GetDependencies() []string {
	return nil
}

func (mo *Module) Start() error {
	if os.Getenv("WAYLAND_DISPLAY") != "" {
		return nil
	}
	logger.Debug("clipboard module start")

	xConn, err := x.NewConn()
	if err != nil {
		return err
	}

	initAtoms(xConn)

	_, err = xfixes.QueryVersion(xConn, xfixes.MajorVersion, xfixes.MinorVersion).Reply(xConn)
	if err != nil {
		logger.Warning(err)
	}

	m := &Manager{}
	m.xc = &xClient{
		conn: xConn,
	}

	err = m.start()
	if err != nil {
		return err
	}

	service := loader.GetService()
	err = service.Export(dbusPath, m)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	return nil
}

func (*Module) Stop() error {
	return nil
}
