// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"deepin-upgrade-manager/pkg/upgrader"
	"flag"
	"fmt"
	"os"
)

const (
	_ACTION_NOTIFY = "notify"
)

var (
	_action = flag.String("action", "", "the available actions: notify")
)

func main() {
	flag.Parse()
	logger.NewLogger("deepin-upgrade-manager-tool", false)
	err := util.FixEnvPath()
	if err != nil {
		logger.Warning("Failed to setenv:", err)
	}
	m := upgrader.NewUpgraderTool()
	switch *_action {
	case _ACTION_NOTIFY:
		if err != nil {
			fmt.Printf("%v", err)
			os.Exit(-1)
		}
		err = m.SendSystemNotice()
		if err != nil {
			fmt.Printf("%v", err)
		}
	}
}
