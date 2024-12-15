// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_mapStrStrEqual(t *testing.T) {
	var test = map[string]string{"aaa": "aaa", "bbb": "bbb", "ccc": "ccc"}
	var test1 = map[string]string{"aaa": "aaa", "bbb": "bbb", "ccc": "ccc"}
	var test2 = map[string]string{"aaa": "aaa", "bbb": "bbb", "ccc": "ddd"}

	assert.Equal(t, mapStrStrEqual(test, test1), true)
	assert.Equal(t, mapStrStrEqual(test, test2), false)
}
