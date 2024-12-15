// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"testing"

	"github.com/godbus/dbus/v5"
	"github.com/stretchr/testify/assert"
)

func Test_getValidName(t *testing.T) {
	names := []string{"BAT0", "test.t", "test:t", "test-t", "test.1:2-3.4:5-6"}
	for _, name := range names {
		path := dbus.ObjectPath("/battery_" + getValidName(name))
		t.Log(path)
		assert.True(t, path.IsValid())
	}
}
