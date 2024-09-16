// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package trayicon

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestDaemon_GetDependencies(t *testing.T) {
	d := Daemon{}
	assert.ElementsMatch(t, []string{}, d.GetDependencies())
}

func TestDaemon_Name(t *testing.T) {
	d := Daemon{}
	assert.Equal(t, moduleName, d.Name())
}

func TestDaemon_Stop(t *testing.T) {
	d := Daemon{}
	assert.Nil(t, d.Stop())
}
