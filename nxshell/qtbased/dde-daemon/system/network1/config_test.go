// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network1

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_loadConfig(t *testing.T) {
	cfg := &Config{}
	err := loadConfig("./testdata/config.json", cfg)
	assert.Nil(t, err)

	err = loadConfig("./testdata/config1.json", cfg)
	assert.NotNil(t, err)
}

func Test_loadConfigSafe(t *testing.T) {
	cfg := loadConfigSafe("./testdata/config.json")
	assert.True(t, cfg.Devices["enp2s0"].Enabled)
	assert.False(t, cfg.VpnEnabled)

	loadConfigSafe("./testdata/config1.json")
}

func Test_saveConfig(t *testing.T) {
	cfg := &Config{}
	err := saveConfig("./testdata/config2.json", cfg)
	assert.Nil(t, err)
}
