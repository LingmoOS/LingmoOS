// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices1

import (
	"testing"

	"github.com/godbus/dbus/v5"
	"github.com/stretchr/testify/assert"
)

func Test_touchscreenSliceEqual(t *testing.T) {
	v1 := []dbus.ObjectPath{
		"/com/lingmo/ABRecovery",
		"/com/lingmo/anything",
	}

	v2 := []dbus.ObjectPath{
		"/com/lingmo/api/Device",
	}
	ok := touchscreenSliceEqual(v1, v2)
	assert.False(t, ok)

	v3 := []dbus.ObjectPath{
		"/com/lingmo/api/Device",
		"/com/lingmo/anything",
	}
	ok = touchscreenSliceEqual(v1, v3)
	assert.False(t, ok)

	ok = touchscreenSliceEqual(v1, v1)
	assert.True(t, ok)
}

func Test_getIndexByDevNode(t *testing.T) {
	m := InputDevices{}
	m.touchscreens = map[dbus.ObjectPath]*Touchscreen{
		"": {
			DevNode: "/com/lingmo/ABRecovery",
		},
	}
	i := m.getIndexByDevNode("")
	assert.Equal(t, -1, i)

	m.touchscreens = map[dbus.ObjectPath]*Touchscreen{
		"/com/lingmo/ABRecovery": {
			DevNode: "/com/lingmo/ABRecovery",
		},
	}
	i = m.getIndexByDevNode("/com/lingmo/ABRecovery")
	assert.Equal(t, -1, i)

	m.Touchscreens = []dbus.ObjectPath{
		"/com/lingmo/ABRecovery",
	}
	i = m.getIndexByDevNode("/com/lingmo/ABRecovery")
	assert.Equal(t, 0, i)
}
