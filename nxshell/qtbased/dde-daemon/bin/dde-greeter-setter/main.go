// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"time"

	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

var logger = log.NewLogger("GreeterSetter")

func main() {
	service, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Errorf("failed to new system service")
		return
	}

	var m = &Manager{
		service: service,
	}

	err = service.Export(dbusPath, m)
	if err != nil {
		logger.Error("failed to export:", err)
		return
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		logger.Error("failed to request name:", err)
		return
	}
	service.SetAutoQuitHandler(time.Second*30, nil)
	service.Wait()
}
