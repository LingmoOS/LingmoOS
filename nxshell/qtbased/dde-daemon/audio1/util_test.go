// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_isVolumeValid(t *testing.T) {
	assert.True(t, isVolumeValid(0))
	assert.False(t, isVolumeValid(-1))
}
func Test_floatPrecision(t *testing.T) {
	assert.Equal(t, floatPrecision(3.1415926), 3.14)
	assert.Equal(t, floatPrecision(2.718281828), 2.72)
}

func Test_toJSON(t *testing.T) {
	str1 := make(map[string]string)
	str1["name"] = "uniontech"
	str1["addr"] = "wuhan"
	ret := toJSON(str1)
	assert.Equal(t, ret, `{"addr":"wuhan","name":"uniontech"}`)
}
