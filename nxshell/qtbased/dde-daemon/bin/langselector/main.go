// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"github.com/linuxdeepin/dde-daemon/langselector1"
	"github.com/linuxdeepin/go-lib/gettext"
)

func main() {
	gettext.InitI18n()
	gettext.BindTextdomainCodeset("dde-daemon", "UTF-8")
	gettext.Textdomain("dde-daemon")
	langselector.Run()
}
