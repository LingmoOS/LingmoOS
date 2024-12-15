// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"github.com/godbus/dbus/v5"
	notifications "github.com/lingmoos/go-dbus-factory/session/org.freedesktop.notifications"
	"github.com/lingmoos/go-lib/dbusutil/proxy"
	"os"

	// system bus
	shutdownfront "github.com/lingmoos/go-dbus-factory/session/com.lingmo.ocean.shutdownfront"
	sensorproxy "github.com/lingmoos/go-dbus-factory/system/net.hadess.sensorproxy"
	daemon "github.com/lingmoos/go-dbus-factory/system/org.lingmo.ocean.daemon1"
	libpower "github.com/lingmoos/go-dbus-factory/system/org.lingmo.ocean.power1"
	ofdbus "github.com/lingmoos/go-dbus-factory/system/org.freedesktop.dbus"
	login1 "github.com/lingmoos/go-dbus-factory/system/org.freedesktop.login1"

	// session bus
	display "github.com/lingmoos/go-dbus-factory/session/org.lingmo.ocean.display1"
	sessionmanager "github.com/lingmoos/go-dbus-factory/session/org.lingmo.ocean.sessionmanager1"
	sessionwatcher "github.com/lingmoos/go-dbus-factory/session/org.lingmo.ocean.sessionwatcher1"
	screensaver "github.com/lingmoos/go-dbus-factory/session/org.freedesktop.screensaver"
	"github.com/lingmoos/go-lib/dbusutil"
	x "github.com/lingmoos/go-x11-client"
)

type Helper struct {
	Notifications notifications.Notifications

	Power         libpower.Power // sig
	LoginManager  login1.Manager // sig
	SensorProxy   sensorproxy.SensorProxy
	SysDBusDaemon ofdbus.DBus
	Daemon        daemon.Daemon

	SessionManager sessionmanager.SessionManager
	SessionWatcher sessionwatcher.SessionWatcher
	ShutdownFront  shutdownfront.ShutdownFront
	ScreenSaver    screensaver.ScreenSaver // sig
	Display        display.Display

	xConn *x.Conn
}

func newHelper(systemBus, sessionBus *dbus.Conn) (*Helper, error) {
	h := &Helper{}
	err := h.init(systemBus, sessionBus)
	if err != nil {
		return nil, err
	}
	return h, nil
}

func (h *Helper) init(sysBus, sessionBus *dbus.Conn) error {
	var err error

	h.Notifications = notifications.NewNotifications(sessionBus)

	h.Power = libpower.NewPower(sysBus)
	h.LoginManager = login1.NewManager(sysBus)
	h.SensorProxy = sensorproxy.NewSensorProxy(sysBus)
	h.SysDBusDaemon = ofdbus.NewDBus(sysBus)
	h.Daemon = daemon.NewDaemon(sysBus)
	h.SessionManager = sessionmanager.NewSessionManager(sessionBus)
	h.ScreenSaver = screensaver.NewScreenSaver(sessionBus)
	h.Display = display.NewDisplay(sessionBus)
	h.SessionWatcher = sessionwatcher.NewSessionWatcher(sessionBus)
	h.ShutdownFront = shutdownfront.NewShutdownFront(sessionBus)

	// init X conn
	if os.Getenv("XDG_SESSION_TYPE") != "wayland" {
		h.xConn, err = x.NewConn()
		if err != nil {
			return err
		}
	}
	return nil
}

func (h *Helper) initSignalExt(systemSigLoop, sessionSigLoop *dbusutil.SignalLoop) {
	// sys
	h.SysDBusDaemon.InitSignalExt(systemSigLoop, true)
	h.LoginManager.InitSignalExt(systemSigLoop, true)
	h.Power.InitSignalExt(systemSigLoop, true)
	h.SensorProxy.InitSignalExt(systemSigLoop, true)
	h.Daemon.InitSignalExt(systemSigLoop, true)
	// session
	h.ScreenSaver.InitSignalExt(sessionSigLoop, true)
	h.SessionWatcher.InitSignalExt(sessionSigLoop, true)
	h.Display.InitSignalExt(sessionSigLoop, true)
}

func (h *Helper) Destroy() {
	h.SysDBusDaemon.RemoveHandler(proxy.RemoveAllHandlers)
	h.LoginManager.RemoveHandler(proxy.RemoveAllHandlers)
	h.Power.RemoveHandler(proxy.RemoveAllHandlers)
	h.SensorProxy.RemoveHandler(proxy.RemoveAllHandlers)
	h.Daemon.RemoveHandler(proxy.RemoveAllHandlers)

	h.ScreenSaver.RemoveHandler(proxy.RemoveAllHandlers)
	h.SessionWatcher.RemoveHandler(proxy.RemoveAllHandlers)

	if h.xConn != nil {
		h.xConn.Close()
		h.xConn = nil
	}
}
