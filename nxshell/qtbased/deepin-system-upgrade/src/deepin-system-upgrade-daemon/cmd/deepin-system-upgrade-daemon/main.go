// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package main

import "deepin-system-upgrade-daemon/pkg/mdbus"

func main() {
	mdbus.Export()
	defer func() {
		mdbus.NotExport()
	}()
	select {}
	return
}
