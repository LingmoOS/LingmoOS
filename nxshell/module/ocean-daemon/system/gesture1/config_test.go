// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package gesture1

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	configPath = "testdata/conf"
)

func Test_loadConfig(t *testing.T) {
	config, err := loadConfig(configPath)
	assert.NoError(t, err)

	assert.Equal(t, config.LongPressDistance, float64(1))
	assert.Equal(t, config.Verbose, 0)
}
