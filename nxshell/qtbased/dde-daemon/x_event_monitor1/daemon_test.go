// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package x_event_monitor

import (
	"testing"

	"github.com/linuxdeepin/go-lib/log"
)

func Test_simpleFunc(t *testing.T) {
	d := Daemon{}

	logger = log.NewLogger(moduleName)
	NewDaemon(logger)

	d.GetDependencies()
	d.Name()
	d.Stop()
}
