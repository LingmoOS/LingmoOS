// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import "github.com/linuxdeepin/dde-daemon/loader"

func init() {
	loader.Register(NewDaemon(logger))
}
