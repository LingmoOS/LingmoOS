// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"time"

	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

//go:generate dbusutil-gen em -type Manager

type Manager struct {
	service    *dbusutil.Service
	writeStart bool
	writeEnd   chan bool
}

const (
	dbusServiceName = "org.deepin.dde.Search1"
	dbusPath        = "/org/deepin/dde/Search1"
	dbusInterface   = dbusServiceName
)

var (
	logger = log.NewLogger("daemon/search")
)

func newManager(service *dbusutil.Service) *Manager {
	m := Manager{
		service: service,
	}

	m.writeStart = false

	return &m
}

func main() {
	logger.BeginTracing()
	defer logger.EndTracing()
	logger.SetRestartCommand("/usr/lib/deepin-daemon/search")

	service, err := dbusutil.NewSessionService()
	if err != nil {
		logger.Fatal("failed to new session service:", err)
	}

	m := newManager(service)
	err = service.Export(dbusPath, m)
	if err != nil {
		logger.Fatal("failed to export:", err)
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		logger.Fatal("failed to request name:", err)
	}

	service.SetAutoQuitHandler(time.Second*5, func() bool {
		if m.writeStart {
			select { //nolint
			case <-m.writeEnd:
				return true
			}
		}
		return true
	})
	service.Wait()
}
