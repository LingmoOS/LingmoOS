// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"flag"
	"time"

	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

var noQuitFlag bool
var logger = log.NewLogger("dde-authority")

func init() {
	flag.BoolVar(&noQuitFlag, "no-quit", false, "do not auto quit")
}

const (
	dbusInterface   = "org.deepin.dde.Authority1"
	dbusServiceName = dbusInterface
	dbusPath        = "/org/deepin/dde/Authority1"

	dbusAgentInterface = dbusInterface + ".Agent"
)

func main() {
	flag.Parse()
	service, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Fatal(err)
	}

	auth := newAuthority(service)
	err = service.Export(dbusPath, auth)
	if err != nil {
		logger.Fatal(err)
	}
	err = service.RequestName(dbusServiceName)
	if err != nil {
		logger.Fatal(err)
	}

	logger.Debug("start service")
	if !noQuitFlag {
		service.SetAutoQuitHandler(3*time.Minute, func() bool {
			auth.mu.Lock()
			canQuit := len(auth.txs) == 0
			auth.mu.Unlock()
			return canQuit
		})
	}
	service.Wait()
}
