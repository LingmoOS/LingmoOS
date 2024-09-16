// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	"github.com/linuxdeepin/dde-daemon/accounts1/logined"
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

var (
	_imageBlur         *ImageBlur
	_userStandardIcons []string
	_accountsManager   *Manager
	_userCustomIcons   []string
	logger             = log.NewLogger("daemon/accounts")
)

func getAccountsManager() *Manager {
	return _accountsManager
}

func init() {
	loader.Register(NewDaemon())
}

type Daemon struct {
	*loader.ModuleBase
	manager        *Manager
	loginedManager *logined.Manager
	imageBlur      *ImageBlur
}

func NewDaemon() *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("accounts", daemon, logger)
	return daemon
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {
	if d.manager != nil {
		return nil
	}

	_userStandardIcons, _userCustomIcons = getUserIcons()
	service := loader.GetService()
	d.manager = NewManager(service)
	_accountsManager = d.manager

	err := service.Export(dbusPath, d.manager)
	if err != nil {
		if d.manager.watcher != nil {
			d.manager.watcher.EndWatch()
			d.manager.watcher = nil
		}
		return err
	}

	d.manager.exportUsers()

	d.imageBlur = newImageBlur(service)
	_imageBlur = d.imageBlur

	err = service.Export(imageBlurDBusPath, d.imageBlur)
	if err != nil {
		d.imageBlur = nil
		return err
	}

	d.loginedManager, err = logined.Register(logger, service)
	if err != nil {
		logger.Error("Failed to create logined manager:", err)
		return err
	}
	err = service.Export(logined.DBusPath, d.loginedManager)
	if err != nil {
		logined.Unregister(d.loginedManager)
		d.loginedManager = nil
		return err
	}

	err = d.manager.initDConfigGreeterWatch()
	if err != nil {
		logger.Warning("init greeter dconfig watch failed:", err)
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	d.manager.initDBusDaemonWatch()
	return nil
}

func (d *Daemon) Stop() error {
	if d.manager != nil {
		d.manager.destroy()
		d.manager = nil
		_accountsManager = nil
	}

	service := loader.GetService()

	if d.imageBlur != nil {
		_ = service.StopExport(d.imageBlur)
		d.imageBlur = nil
		_imageBlur = nil
	}

	if d.loginedManager != nil {
		_ = service.StopExport(d.loginedManager)
		d.loginedManager = nil
	}

	return nil
}
