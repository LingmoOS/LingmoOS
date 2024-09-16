// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package display1

import (
	"os"
	"os/exec"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_isInVM(t *testing.T) {
	err := exec.Command("systemd-detect-virt", "-v", "-q").Run()
	if err != nil {
		assert.False(t, isInVM())
	} else {
		assert.True(t, isInVM())
	}
}

func Test_binExist(t *testing.T) {
	const cmdPath = "/usr/bin/apt"
	_, err := os.Stat(cmdPath)
	if err != nil {
		if os.IsNotExist(err) {
			assert.False(t, binExist(cmdPath))
		}
	} else {
		assert.True(t, binExist(cmdPath))
	}
}

func Test_loadRendererConfig(t *testing.T) {
	const cfgPath = "./test_data"

	var cfg RendererConfig
	cfg.BlackList = []string{
		"llvmpipe",
	}
	err := genRendererConfig(&cfg, cfgPath)
	assert.Nil(t, err)
	defer func() {
		_ = os.RemoveAll(cfgPath)
	}()
	c, err := loadRendererConfig(cfgPath)
	assert.Nil(t, err)
	assert.NotNil(t, c)
}

func Test_genRendererConfig(t *testing.T) {
	const cfgPath = "./test_data"

	var cfg RendererConfig
	cfg.BlackList = []string{
		"llvmpipe",
	}
	err := genRendererConfig(&cfg, cfgPath)
	assert.Nil(t, err)
	_ = os.RemoveAll(cfgPath)
}
