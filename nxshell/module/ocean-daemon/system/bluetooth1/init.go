// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"github.com/lingmoos/ocean-daemon/loader"
	"github.com/lingmoos/go-lib/log"
)

var logger = log.NewLogger("daemon/bluetooth")

func init() {
	loader.Register(newBluetoothModule(logger))
}
