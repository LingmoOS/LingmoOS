// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package lastore

import (
	"sync"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/common/dsync"
	eventLog "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.EventLog1"
	network "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.network1"
	notifications "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.notifications"
	lastore "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.lastore1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

//go:generate dbusutil-gen em -type Lastore

type Lastore struct {
	service        *dbusutil.Service
	sysSigLoop     *dbusutil.SignalLoop
	sessionSigLoop *dbusutil.SignalLoop

	core          lastore.Lastore
	notifications notifications.Notifications
	eventLog      eventLog.EventLog

	syncConfig *dsync.Config

	network network.Network
	// prop:
	PropsMu sync.RWMutex
}

func newLastore(service *dbusutil.Service) (*Lastore, error) {
	l := &Lastore{
		service: service,
	}
	l.network = network.NewNetwork(service.Conn())
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		return nil, err
	}

	l.sysSigLoop = dbusutil.NewSignalLoop(systemBus, 100)
	l.sysSigLoop.Start()

	l.sessionSigLoop = dbusutil.NewSignalLoop(sessionBus, 10)
	l.sessionSigLoop.Start()

	l.initCore(systemBus)
	l.initNotify(sessionBus)
	l.initEventLog(sessionBus)

	l.syncConfig = dsync.NewConfig("updater", &syncConfig{l: l}, l.sessionSigLoop, dbusPath, logger)
	return l, nil
}

func (l *Lastore) initNotify(sessionBus *dbus.Conn) {
	l.notifications = notifications.NewNotifications(sessionBus)
}

func (l *Lastore) initCore(systemBus *dbus.Conn) {
	l.core = lastore.NewLastore(systemBus)
}

func (l *Lastore) initEventLog(sessionBus *dbus.Conn) {
	l.eventLog = eventLog.NewEventLog(sessionBus)
}

func (l *Lastore) destroy() {
	l.sysSigLoop.Stop()
	l.syncConfig.Destroy()
}

func (l *Lastore) GetInterfaceName() string {
	return "com.deepin.LastoreSessionHelper"
}

func (*Lastore) IsDiskSpaceSufficient() (result bool, busErr *dbus.Error) {
	avail, err := queryVFSAvailable("/")
	if err != nil {
		return false, dbusutil.ToError(err)
	}
	return avail > 1024*1024*10 /* 10 MB */, nil
}
