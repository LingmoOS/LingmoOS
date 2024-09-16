// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package soundeffect

import (
	"time"

	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gsettings"
	"github.com/linuxdeepin/go-lib/log"
)

var logger = log.NewLogger("daemon/soundeffect")

func run() error {
	service, err := dbusutil.NewSessionService()
	if err != nil {
		return err
	}
	m := NewManager(service)
	err = m.init()
	if err != nil {
		return err
	}

	_ = gsettings.StartMonitor()

	serverObj, err := service.NewServerObject(dbusPath, m)
	if err != nil {
		return err
	}

	err = serverObj.SetWriteCallback(m, "Enabled", m.enabledWriteCb)
	if err != nil {
		return err
	}
	err = serverObj.Export()
	if err != nil {
		return err
	}

	err = service.RequestName(DBusServiceName)
	if err != nil {
		return err
	}

	service.SetAutoQuitHandler(30*time.Second, m.canQuit)
	service.Wait()
	return nil
}

func Run() {
	err := run()
	if err != nil {
		logger.Fatal(err)
	}
}
