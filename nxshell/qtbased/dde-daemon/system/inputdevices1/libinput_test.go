// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices1

import (
	"testing"

	"github.com/linuxdeepin/go-lib/log"
	"github.com/stretchr/testify/assert"
)

func Test_newLibinput(t *testing.T) {
	inputDevices := &InputDevices{}
	logger.SetLogLevel(log.LevelInfo)
	l := newLibinput(inputDevices)

	logger.SetLogLevel(log.LevelDebug)
	l = newLibinput(inputDevices)
	assert.NotNil(t, l)
}
