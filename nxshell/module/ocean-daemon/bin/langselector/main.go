// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"github.com/lingmoos/ocean-daemon/langselector1"
	"github.com/lingmoos/go-lib/gettext"
)

func main() {
	gettext.InitI18n()
	gettext.BindTextdomainCodeset("ocean-daemon", "UTF-8")
	gettext.Textdomain("ocean-daemon")
	langselector.Run()
}
