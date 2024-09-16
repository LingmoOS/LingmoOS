// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"deepin-upgrade-manager/pkg/bootkit"
	config "deepin-upgrade-manager/pkg/config/bootkit"
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"flag"
	"fmt"
	"os"
	"strings"
)

const (
	_ACTION_LIST         = "list"
	_ACTION_UPDATE       = "update"
	_ACTION_VERSION      = "version"
	_ACTION_MKGRUBCONFIG = "mkgrubconfig"
	_ACTION_MKINITRD     = "mkinitrd"
)

var (
	_config = flag.String("config", "/usr/share/deepin-boot-kit/config.json", "the configuration file path")
	_action = flag.String("action", "list", "the available actions: version, list, update, mkgrubconfig, mkinitrd")
)

func main() {
	flag.Parse()

	conf, err := config.BootLoadConfig(*_config)
	if err != nil {
		fmt.Println("load config wrong:", err)
		os.Exit(-1)
	}
	err = conf.BootPrepare()
	if err != nil {
		fmt.Println("prepare config wrong:", err)
		os.Exit(-1)
	}
	if os.Geteuid() != 0 {
		logger.Info("Must run with privileged user")
		os.Exit(-1)
	}
	err = util.FixEnvPath()
	if err != nil {
		logger.Warning("Failed to setenv:", err)
	}
	logger.NewLogger("deepin-boot-kit", false)
	m, err := bootkit.NewBootkit(conf)
	if err != nil {
		logger.Fatal("Failed to new bootkit:", err)
		os.Exit(-1)
	}
	handleAction(m, conf)
}

func handleAction(m *bootkit.Bootkit, c *config.BootConfig) {
	switch *_action {
	case _ACTION_LIST:
		m.InitVersionInfo()
		verList := m.ListVersion()
		fmt.Printf("AvailVersionList:%s\n", strings.Join(verList, " "))
	case _ACTION_VERSION:
		version := m.GetNewVersion()
		if len(version) == 0 {
			logger.Error("the version cannot be empty")
			os.Exit(-1)
		}
		fmt.Printf("%s", version)
	case _ACTION_UPDATE:
		err := m.UpdateGrub()
		if err != nil {
			os.Exit(-1)
		}
	case _ACTION_MKGRUBCONFIG:
		m.InitVersionInfo()
		out := m.GenerateDefaultGrub()
		fmt.Println(out)
	case _ACTION_MKINITRD:
		needUpdate := os.Getenv("INITRAMFS_UPDATE")
		if needUpdate == "n" {
			m.CopyToolScripts()
		} else {
			m.UpdateInitramfs()
		}

	}
}
