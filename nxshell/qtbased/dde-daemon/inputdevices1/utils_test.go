// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_addItemToList(t *testing.T) {
	dst := []string{}
	src := []string{
		"hello",
		"world",
		"foo",
		"bar",
	}
	var ret bool

	for _, str := range src {
		dst, ret = addItemToList(str, dst)
		assert.True(t, ret)
	}

	for _, str := range src {
		dst, ret = addItemToList(str, dst)
		assert.False(t, ret)
	}

	for _, str := range src {
		dst, ret = delItemFromList(str, dst)
		assert.True(t, ret)
	}

	for _, str := range src {
		dst, ret = delItemFromList(str, dst)
		assert.False(t, ret)
	}
}
