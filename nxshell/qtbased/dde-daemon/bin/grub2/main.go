// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"flag"
	"fmt"
	"os"

	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/dde-daemon/grub2"
)

var logger = log.NewLogger("daemon/grub2")

func init() {
	grub2.SetLogger(logger)
}

var (
	optPrepareGfxmodeDetect bool
	optSetupTheme           bool
	optDebug                bool
	optOSNum                bool
)

func main() {
	flag.BoolVar(&optDebug, "debug", false, "debug mode")
	flag.BoolVar(&optPrepareGfxmodeDetect, "prepare-gfxmode-detect", false,
		"prepare gfxmode detect")
	flag.BoolVar(&optSetupTheme, "setup-theme", false, "do nothing")
	flag.BoolVar(&optOSNum, "os-num", false, "get system num")
	flag.Parse()
	if optDebug {
		logger.SetLogLevel(log.LevelDebug)
	}

	if optPrepareGfxmodeDetect {
		logger.Debug("mode: prepare gfxmode detect")
		err := grub2.PrepareGfxmodeDetect()
		if err != nil {
			logger.Warning(err)
			os.Exit(2)
		}
	} else if optSetupTheme {
		// for compatibility
		return
	} else if optOSNum {
		num, err := grub2.GetOSNum()
		if err != nil {
			logger.Warning(err)
			os.Exit(2)
		}
		fmt.Println(num)
	} else {
		logger.Debug("mode: daemon")
		grub2.RunAsDaemon()
	}
}
