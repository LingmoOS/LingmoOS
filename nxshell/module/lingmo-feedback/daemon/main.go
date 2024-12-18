/**
 * Copyright (C) 2015 Lingmo Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

package main

import (
	dbusOrigIfc "dbus/org/freedesktop/dbus/system"
	"os"
	"pkg.lingmo.io/lib"
	"pkg.lingmo.io/lib/dbus"
	. "pkg.lingmo.io/lib/gettext"
	"pkg.lingmo.io/lib/log"
	"time"
)

var logger = log.NewLogger(dbusDest)
var dbusDaemon, _ = dbusOrigIfc.NewDBusDaemon("org.freedesktop.DBus", "/org/freedesktop/DBus")

func main() {
	InitI18n()
	Textdomain("lingmo-feedback")
	initCategories()

	if !lib.UniqueOnSystem(dbusDest) {
		logger.Warning("dbus interface already exists", dbusDest)
		return
	}
	fd := NewFeedbackDaemon()
	if err := dbus.InstallOnSystem(fd); err != nil {
		logger.Error("register dbus interface failled", err)
		return
	}

	dbus.SetAutoDestroyHandler(60*time.Second, func() bool {
		return !fd.isInWorking()
	})
	dbus.DealWithUnhandledMessage()
	if err := dbus.Wait(); err != nil {
		logger.Error("lost dbus session", err)
		os.Exit(1)
	}
}
