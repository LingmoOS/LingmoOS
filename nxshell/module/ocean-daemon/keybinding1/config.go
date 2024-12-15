// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"encoding/json"
	"os"
)

type Config struct {
	HandleTouchPadToggle bool
}

var globalConfig Config

func loadConfig() {
	data, err := os.ReadFile("/var/lib/ocean-daemon/keybinding/config.json")
	if err != nil {
		if !os.IsNotExist(err) {
			logger.Warning(err)
		}
		return
	}

	err = json.Unmarshal(data, &globalConfig)
	if err != nil {
		logger.Warning(err)
	}

	logger.Debugf("loadConfig %#v", globalConfig)
}
