// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

// #cgo CXXFLAGS:-O2 -std=c++11
// #cgo CFLAGS: -W -Wall -fstack-protector-all -fPIC
// #cgo LDFLAGS:-ldl
// #include <stdlib.h>
// #include "event_sdk.h"
import "C"
import (
	"errors"
	"fmt"
	"sync"
	"unsafe"

	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"

	"github.com/linuxdeepin/dde-daemon/loader"
)

var logger = log.NewLogger("daemon/session/eventlog")
var _collectorMap = make(map[string]BaseCollector)

type writeEventLogFunc func(msg string)

func init() {
	loader.Register(newModule(logger))
}

var _collectorMapMu sync.Mutex

func register(name string, c BaseCollector) {
	_collectorMapMu.Lock()
	defer _collectorMapMu.Unlock()
	_collectorMap[name] = c
}

type Module struct {
	*loader.ModuleBase
	eventlog *EventLog
	writeMu  sync.Mutex
}

func (m *Module) GetDependencies() []string {
	return []string{"dock"}
}

func newModule(logger *log.Logger) *Module {
	m := new(Module)
	m.ModuleBase = loader.NewModuleBase("eventlog", m, logger)
	return m
}

func (m *Module) Start() error {
	err := start()
	if err != nil {
		logger.Warning(err)
		return nil
	}
	service := loader.GetService()
	m.eventlog = newEventLog(service, m.writeEventLog)
	if m.eventlog == nil {
		return errors.New("failed to create eventlog")
	}
	err = service.Export(dbusPath, m.eventlog)
	if err != nil {
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}
	err = m.eventlog.start()
	if err != nil {
		logger.Warning(err)
		return err
	}

	for _, c := range _collectorMap {
		err := c.Init(service, m.writeEventLog)
		if err != nil {
			logger.Warning(err)
			continue
		}
		err = c.Collect()
		if err != nil {
			logger.Warning(err)
		}
	}

	return nil
}

func start() error {
	session, err := dbusutil.NewSessionService()
	if err != nil {
		return err
	}
	has, err := session.NameHasOwner("com.deepin.userexperience.Daemon")
	if err != nil {
		return err
	}
	if has {
		return errors.New("do not need to start eventlog")
	}
	ret := C.InitEventSDK()
	if ret != 0 {
		err := fmt.Errorf("failed to initialize event SDK:%d", ret)
		return err
	}
	return nil
}

func (m *Module) Stop() error {
	for _, c := range _collectorMap {
		err := c.Stop()
		if err != nil {
			logger.Warning(err)
		}
	}
	m.eventlog.sessionSigLoop.Stop()
	return stop()
}

func stop() error {
	C.CloseEventLog()
	_collectorMap = make(map[string]BaseCollector)
	return nil
}

func (m *Module) writeEventLog(msg string) {
	m.writeMu.Lock()
	defer m.writeMu.Unlock()
	cStr := C.CString(msg)
	defer C.free(unsafe.Pointer(cStr))
	var isDebug int
	if logger.GetLogLevel() == log.LevelDebug {
		isDebug = 1
	} else {
		isDebug = 0
	}
	C.writeEventLog(cStr, C.int(isDebug))
}
