// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_SystemLayout(t *testing.T) {
	layout, err := getSystemLayout("testdata/keyboard")
	assert.NoError(t, err)
	assert.Equal(t, layout, "us;")
}

func Test_ParseXKBFile(t *testing.T) {
	handler, err := getLayoutsFromFile("testdata/base.xml")
	assert.NoError(t, err)
	assert.NotNil(t, handler)
}

func Test_StrList(t *testing.T) {
	var list = []string{"abc", "xyz", "123"}
	ret, added := addItemToList("456", list)
	assert.Len(t, ret, 4)
	assert.True(t, added)

	ret, added = addItemToList("123", list)
	assert.Len(t, ret, 3)
	assert.False(t, added)

	ret, deleted := delItemFromList("123", list)
	assert.Equal(t, len(ret), 2)
	assert.True(t, deleted)

	ret, deleted = delItemFromList("456", list)
	assert.Len(t, ret, 3)
	assert.False(t, deleted)

	assert.True(t, isItemInList("123", list))
	assert.False(t, isItemInList("456", list))
}

func Test_SyndaemonExist(t *testing.T) {
	assert.False(t, isSyndaemonExist("testdata/syndaemon.pid"))
	assert.True(t, isProcessExist("testdata/dde-desktop-cmdline", "dde-desktop"))
}

func Test_CurveControlPoints(t *testing.T) {
	// output svg path for debug
	for i := 1; i <= 7; i++ {
		p := getPressureCurveControlPoints(i)
		fmt.Printf(
			`<path d="M0,0 C%v,%v %v,%v 100,100" stroke="red" fill="none" style="stroke-width: 2px;"></path>`,
			p[0], p[1], p[2], p[3])
		fmt.Println("")
	}
}
