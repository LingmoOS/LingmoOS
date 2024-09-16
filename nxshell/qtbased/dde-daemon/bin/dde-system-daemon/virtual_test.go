// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

var testVirtuals []string = []string{"hvm", "virtual", "vmware", "kvm", "cloud", "invented"}

func TestGetValidSupData(t *testing.T) {
	t.Run("Test file get Valid Support Data check", func(t *testing.T) {
		assert.Equal(t, []string{"invented", "cloud", "kvm", "vmware", "virtual", "bochs", "hvm"}, getValidSupData([]string{"hvm", "bochs", "virtual", "vmware", "kvm", "cloud", "invented", ""}))
	})
}

func Test_isVirtual(t *testing.T) {
	t.Run("Test file isVirtual check", func(t *testing.T) {
		assert.Equal(t, true, isVirtual("/opt/apps/com.huawei.fusionaccessclient/hdpclienthwcloud", testVirtuals))
		assert.Equal(t, true, isVirtual("/opt/appshvm", testVirtuals))
		assert.Equal(t, true, isVirtual("/opt/apps/com.huawei.fusionaccessclient/virtual", testVirtuals))
		assert.Equal(t, true, isVirtual("/opt/apps/vmware", testVirtuals))
		assert.Equal(t, true, isVirtual("/opt/apps/invented", testVirtuals))
		assert.Equal(t, true, isVirtual("/opt/apps/kvm", testVirtuals))
		assert.Equal(t, false, isVirtual("/test/data", testVirtuals))
	})
}
