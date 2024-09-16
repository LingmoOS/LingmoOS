// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"time"

	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/dde-daemon/network1/proxychains"
	"github.com/linuxdeepin/go-lib/log"
	libnotify "github.com/linuxdeepin/go-lib/notify"
)

var (
	logger  = log.NewLogger("daemon/network")
	manager *Manager
)

func init() {
	loader.Register(newModule(logger))
	proxychains.SetLogger(logger)
}

func HandlePrepareForSleep(sleep bool) {
	if manager == nil {
		logger.Warning("Module 'network' has not start")
		return
	}
	if sleep {
		// suspend
		disableNotify()
		return
	}
	// wakeup
	enableNotify()
	//value decided the strategy of the wirelessScan
	_ = manager.RequestWirelessScan()
	time.AfterFunc(3*time.Second, func() {
		manager.clearAccessPoints()
	})
}

type Module struct {
	*loader.ModuleBase
}

func newModule(logger *log.Logger) *Module {
	module := new(Module)
	module.ModuleBase = loader.NewModuleBase("network", module, logger)
	return module
}

func (d *Module) GetDependencies() []string {
	return []string{}
}

func (d *Module) start() error {
	service := loader.GetService()
	manager = NewManager(service)
	manager.init()

	managerServerObj, err := service.NewServerObject(dbusPath, manager, manager.syncConfig)
	if err != nil {
		return err
	}

	err = managerServerObj.SetWriteCallback(manager, "NetworkingEnabled", manager.networkingEnabledWriteCb)
	if err != nil {
		return err
	}
	err = managerServerObj.SetWriteCallback(manager, "VpnEnabled", manager.vpnEnabledWriteCb)
	if err != nil {
		return err
	}

	err = managerServerObj.Export()
	if err != nil {
		logger.Error("failed to export manager:", err)
		manager = nil
		return err
	}

	manager.proxyChainsManager = proxychains.NewManager(service)
	err = service.Export(proxychains.DBusPath, manager.proxyChainsManager)
	if err != nil {
		logger.Warning("failed to export proxyChainsManager:", err)
		manager.proxyChainsManager = nil
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	err = manager.syncConfig.Register()
	if err != nil {
		logger.Warning("Failed to register sync service:", err)
	}

	initDBusDaemon()
	watchNetworkManagerRestart(manager)
	return nil
}

func (d *Module) Start() error {
	libnotify.Init("dde-session-daemon")
	if manager != nil {
		return nil
	}

	initSlices() // initialize slice code
	initSysSignalLoop()
	initNotifyManager()
	return d.start()
}

func (d *Module) Stop() error {
	if manager == nil {
		return nil
	}

	service := loader.GetService()

	err := service.ReleaseName(dbusServiceName)
	if err != nil {
		logger.Warning(err)
	}

	manager.destroy()
	destroyDBusDaemon()
	sysSigLoop.Stop()
	err = service.StopExport(manager)
	if err != nil {
		logger.Warning(err)
	}

	if manager.proxyChainsManager != nil {
		err = service.StopExport(manager.proxyChainsManager)
		if err != nil {
			logger.Warning(err)
		}
		manager.proxyChainsManager = nil
	}

	manager = nil
	return nil
}
