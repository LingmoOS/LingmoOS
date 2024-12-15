// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"os"

	// modules:
	_ "github.com/lingmoos/ocean-daemon/accounts1"
	_ "github.com/lingmoos/ocean-daemon/apps1"
	_ "github.com/lingmoos/ocean-daemon/fprintd1"
	_ "github.com/lingmoos/ocean-daemon/image_effect1"
	_ "github.com/lingmoos/ocean-daemon/system/airplane_mode1"
	_ "github.com/lingmoos/ocean-daemon/system/bluetooth1"
	_ "github.com/lingmoos/ocean-daemon/system/display1"
	_ "github.com/lingmoos/ocean-daemon/system/gesture1"
	_ "github.com/lingmoos/ocean-daemon/system/hostname"
	_ "github.com/lingmoos/ocean-daemon/system/inputdevices1"
	_ "github.com/lingmoos/ocean-daemon/system/keyevent1"
	_ "github.com/lingmoos/ocean-daemon/system/lang"
	_ "github.com/lingmoos/ocean-daemon/system/network1"
	_ "github.com/lingmoos/ocean-daemon/system/power1"
	_ "github.com/lingmoos/ocean-daemon/system/power_manager1"
	_ "github.com/lingmoos/ocean-daemon/system/resource_ctl"
	_ "github.com/lingmoos/ocean-daemon/system/scheduler"
	_ "github.com/lingmoos/ocean-daemon/system/swapsched1"
	_ "github.com/lingmoos/ocean-daemon/system/systeminfo1"
	_ "github.com/lingmoos/ocean-daemon/system/timedate1"
	_ "github.com/lingmoos/ocean-daemon/system/uadp1"
	systemd1 "github.com/lingmoos/go-dbus-factory/system/org.freedesktop.systemd1"

	"github.com/lingmoos/ocean-daemon/loader"
	login1 "github.com/lingmoos/go-dbus-factory/system/org.freedesktop.login1"
	glib "github.com/lingmoos/go-gir/glib-2.0"
	"github.com/lingmoos/go-lib/dbusutil"
	. "github.com/lingmoos/go-lib/gettext"
	"github.com/lingmoos/go-lib/log"
)

//go:generate dbusutil-gen em -type Daemon

type Daemon struct {
	loginManager  login1.Manager
	systemSigLoop *dbusutil.SignalLoop
	service       *dbusutil.Service
	systemd       systemd1.Manager
	signals       *struct { // nolint
		HandleForSleep struct {
			start bool
		}
	}
}

const (
	dbusServiceName = "org.lingmo.ocean.Daemon1"
	dbusPath        = "/org/lingmo/ocean/Daemon1"
	dbusInterface   = dbusServiceName
)

var logger = log.NewLogger("daemon/ocean-system-daemon")
var _daemon *Daemon

func main() {
	service, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Fatal("failed to new system service", err)
	}

	hasOwner, err := service.NameHasOwner(dbusServiceName)
	if err != nil {
		logger.Fatal("failed to call NameHasOwner:", err)
	}
	if hasOwner {
		logger.Warningf("name %q already has the owner", dbusServiceName)
		os.Exit(1)
	}

	// fix no PATH when was launched by dbus
	if os.Getenv("PATH") == "" {
		logger.Warning("No PATH found, manual special")
		err = os.Setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin")
		if err != nil {
			logger.Warning(err)
		}
	}

	// 系统级服务，无需设置LANG和LANGUAGE，保证翻译不受到影响
	_ = os.Setenv("LANG", "")
	_ = os.Setenv("LANGUAGE", "")

	InitI18n()
	BindTextdomainCodeset("ocean-daemon", "UTF-8")
	Textdomain("ocean-daemon")

	logger.SetRestartCommand("/usr/lib/lingmo-daemon/ocean-system-daemon")

	_daemon = &Daemon{
		loginManager:  login1.NewManager(service.Conn()),
		service:       service,
		systemSigLoop: dbusutil.NewSignalLoop(service.Conn(), 10),
		systemd:       systemd1.NewManager(service.Conn()),
	}
	_daemon.service = service
	err = service.Export(dbusPath, _daemon)
	if err != nil {
		logger.Fatal("failed to export:", err)
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		logger.Fatal("failed to request name:", err)
	}

	startBacklightHelperAsync(service.Conn())
	loader.SetService(service)
	loader.StartAll()
	defer loader.StopAll()

	// NOTE: system/power module requires glib loop
	go glib.StartLoop()
	_daemon.systemSigLoop.Start()
	err = _daemon.forwardPrepareForSleepSignal(service)
	if err != nil {
		logger.Warning(err)
	}
	service.Wait()
}

func (*Daemon) GetInterfaceName() string {
	return dbusInterface
}
