// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"context"
	"time"

	dbus "github.com/godbus/dbus/v5"
	ipwatchd "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.ipwatchd1"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

func activateSystemService(sysBus *dbus.Conn, serviceName string) error {
	sysBusObj := ofdbus.NewDBus(sysBus)

	has, err := sysBusObj.NameHasOwner(0, serviceName)
	if err != nil {
		return err
	}

	if has {
		logger.Debug("service activated", serviceName)
		return nil
	}
	_, err = sysBusObj.StartServiceByName(0, serviceName, 0)
	if err != nil {
		return err
	}

	return nil
}

func (m *Manager) initIPConflictManager(sysBus *dbus.Conn) {
	m.sysIPWatchD = ipwatchd.NewIPWatchD(sysBus)
	m.sysIPWatchD.InitSignalExt(m.sysSigLoop, true)
	err := activateSystemService(sysBus, m.sysIPWatchD.ServiceName_())
	if err != nil {
		logger.Warning(err)
	}

	_, err = m.sysIPWatchD.ConnectIPConflict(func(ip, smac, dmac string) {
		err := m.service.Emit(manager, "IPConflict", ip, dmac)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) RequestIPConflictCheck(ip, ifc string) *dbus.Error {
	conn, err := dbus.SystemBus()
	if err != nil {
		return dbusutil.ToError(err)
	}

	go func() {
		ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
		defer cancel()

		var mac string
		err = conn.Object("com.deepin.dde.IPWatchD1", "org/deepin/dde/IPWatchD1").CallWithContext(ctx, "org.deepin.dde.IPWatchD1.RequestIPConflictCheck", 0, ip, ifc).Store(&mac)
		if err != nil {
			logger.Warning(err)
		}
		logger.Debug("send ip conflict check result: ", ip, mac)
		m.service.Emit(manager, "IPConflict", ip, mac)
	}()

	return nil
}
