// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import (
	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"strings"
	"sync"

	"github.com/linuxdeepin/dde-daemon/loader"
)

var logger = log.NewLogger("daemon/systeminfo")

func init() {
	loader.Register(NewModule())
}

type Module struct {
	m *Manager
	*loader.ModuleBase
}

func (m *Module) GetDependencies() []string {
	return nil
}

func (m *Module) Start() error {
	if m.m != nil {
		return nil
	}
	logger.Debug("system info module start")
	service := loader.GetService()
	m.m = NewManager(service)
	serverObj, err := service.NewServerObject(dbusPath, m.m)
	if err != nil {
		logger.Warning(err)
		return err
	}

	handleMem := func() {
		// init get memory
		err = m.m.calculateMemoryViaLshw()
		if err != nil {
			logger.Warning(err)
		}
	}
	var memOnce sync.Once
	err = serverObj.SetReadCallback(m.m, "MemorySize", func(read *dbusutil.PropertyRead) *dbus.Error {
		memOnce.Do(func() {
			handleMem()
		})
		return nil
	})
	if err != nil {
		logger.Warning(err)
	}
	err = serverObj.SetReadCallback(m.m, "MemorySizeHuman", func(read *dbusutil.PropertyRead) *dbus.Error {
		memOnce.Do(func() {
			handleMem()
		})
		return nil
	})
	if err != nil {
		logger.Warning(err)
	}

	handleDisplay := func() {
		// get system display driver info
		classContent, err := getLshwData("display")
		if err != nil {
			logger.Warning(err)
		} else {
			var res []string
			for _, item := range classContent {
				res = append(res, item.Configuration.Driver)
			}
			m.m.setPropDisplayDriver(strings.Join(res, "&&"))
		}
	}
	var displayOnce sync.Once
	err = serverObj.SetReadCallback(m.m, "DisplayDriver", func(read *dbusutil.PropertyRead) *dbus.Error {
		displayOnce.Do(func() {
			handleDisplay()
		})
		return nil
	})
	if err != nil {
		logger.Warning(err)
	}

	handleVideo := func() {
		// get system video driver info
		classContent, err := getLshwData("video")
		if err != nil {
			logger.Warning(err)
		} else {
			var res []string
			for _, item := range classContent {
				res = append(res, item.Configuration.Driver)
			}
			m.m.setPropVideoDriver(strings.Join(res, "&&"))
		}
	}
	var videoOnce sync.Once
	err = serverObj.SetReadCallback(m.m, "VideoDriver", func(read *dbusutil.PropertyRead) *dbus.Error {
		videoOnce.Do(func() {
			handleVideo()
		})
		return nil
	})
	if err != nil {
		logger.Warning(err)
	}

	handleCurrentSpeed := func() {
		// get system bit
		systemType := 64
		if "64" != m.m.systemBit() {
			systemType = 32
		}
		// Get CPU MHZ
		currentSpeed, err1 := GetCurrentSpeed(systemType)
		if err1 != nil {
			logger.Warning(err1)
			return
		}
		m.m.setPropCurrentSpeed(currentSpeed)
	}

	// speed是通过dmidecode获取，不影响性能
	handleCurrentSpeed()

	err = serverObj.Export()
	if err != nil {
		logger.Warning(err)
		return err
	}
	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	return nil
}

func (m *Module) Stop() error {
	return nil
}

func NewModule() *Module {
	m := &Module{}
	m.ModuleBase = loader.NewModuleBase("systeminfo", m, logger)
	return m
}
