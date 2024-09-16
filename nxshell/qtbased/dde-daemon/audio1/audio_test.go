// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"testing"

	"github.com/stretchr/testify/assert"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/pulse"
)

func Test_objectPathSliceEqual(t *testing.T) {
	var str = []dbus.ObjectPath{"/org/deepin/dde/Bluetooth1", "/org/deepin/dde/Audio1"}
	var str1 = []dbus.ObjectPath{"/org/deepin/dde/Bluetooth1", "/org/deepin/dde/Audio1"}
	var str2 = []dbus.ObjectPath{"/org/deepin/dde/Bluetooth1", "/org/deepin/dde/Audio1", "/"}
	var str3 = []dbus.ObjectPath{"/org/deepin/dde/Bluetooth1", "/org/deepin/dde/Accounts1"}

	assert.Equal(t, objectPathSliceEqual(str, str1), true)
	assert.Equal(t, objectPathSliceEqual(str, str2), false)
	assert.Equal(t, objectPathSliceEqual(str, str3), false)
}

func Test_isPortExists(t *testing.T) {
	var tests = []pulse.PortInfo{
		{Name: "MG-T1S", Description: "audio", Priority: 1, Available: pulse.AvailableTypeUnknow},
		{Name: "MG-T1S", Description: "audio", Priority: 2, Available: pulse.AvailableTypeNo},
		{Name: "MG-T1S", Description: "audio", Priority: 3, Available: pulse.AvailableTypeYes},
		{Description: "audio", Priority: 4, Available: pulse.AvailableTypeUnknow},
		{Description: "audio", Priority: 5, Available: pulse.AvailableTypeNo},
		{Description: "audio", Priority: 6, Available: pulse.AvailableTypeYes},
		{Name: "MG-T1S", Description: "audio", Priority: 7, Available: pulse.AvailableTypeUnknow},
		{Name: "MG-T1S", Description: "audio", Priority: 8, Available: pulse.AvailableTypeNo},
		{Name: "MG-T1S", Description: "audio", Priority: 9, Available: pulse.AvailableTypeYes},
	}
	assert.Equal(t, isPortExists("MG-T1S", tests), true)
	assert.Equal(t, isPortExists("", tests), true)
	assert.Equal(t, isPortExists("MG", tests), false)
}

func Test_getBestPort(t *testing.T) {
	var tests = []pulse.PortInfo{
		{Name: "MG-T1S", Description: "audio", Priority: 1, Available: pulse.AvailableTypeUnknow},
		{Name: "MG-T1S", Description: "audio", Priority: 2, Available: pulse.AvailableTypeNo},
		{Name: "MG-T1S", Description: "audio", Priority: 3, Available: pulse.AvailableTypeYes},
		{Description: "audio", Priority: 4, Available: pulse.AvailableTypeUnknow},
		{Description: "audio", Priority: 5, Available: pulse.AvailableTypeNo},
		{Description: "audio", Priority: 6, Available: pulse.AvailableTypeYes},
		{Name: "MG-T1S", Description: "audio", Priority: 7, Available: pulse.AvailableTypeUnknow},
		{Name: "MG-T1S", Description: "audio", Priority: 8, Available: pulse.AvailableTypeNo},
		{Name: "MG-T1S", Description: "audio", Priority: 9, Available: pulse.AvailableTypeYes},
	}
	var ret = pulse.PortInfo{Name: "MG-T1S", Description: "audio", Priority: 9, Available: pulse.AvailableTypeYes}
	ret1 := getBestPort(tests)
	assert.Equal(t, ret1, ret)
}

func Test_isStrvEqual(t *testing.T) {
	var str = []string{"test1", "test2", "test3", "test4"}
	var str1 = []string{"test1", "test2", "test4", "test3"}
	var str2 = []string{"test1", "test2", "test3", "Test4"}

	assert.Equal(t, isStrvEqual(str, str1), true)
	assert.Equal(t, isStrvEqual(str, str2), false)

}
